#include <iostream>
#include "master_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "scene_scheduler_static.hpp"
#include "scene_scheduler_dynamic.hpp"

namespace
{
Master *g_pMaster;
SchedulingPolicyParam g_SchedulingPolicyParam;
}


SchedulingPolicyParam& SchedulingPolicyParam::Get()
{
    return g_SchedulingPolicyParam;
}

MasterSchedulerThread::MasterSchedulerThread(int scheduling_thread_q_depth) : MsgQThread("MasterSchedulerThread", scheduling_thread_q_depth)
{
}

void MasterSchedulerThread::Start()
{
    DEBUG_TRACE("Tid: " << std::hex << std::this_thread::get_id() << ", MasterSchedulerThread::Start(" << std::hex << this << ")");
    m_thread = new std::thread(&MasterSchedulerThread::Run, *this);
}

void MasterSchedulerThread::Run()
{
    while (1)
    {
        MsgQEntry msgQEntry = TakeNext();
        RELEASE_TRACE("Tid: " << std::hex << std::this_thread::get_id() << ", MasterSchedulerThread::Run(" << std::hex << this << ") - Received MsgId: "
                      << msgQEntry.m_Msg->GetId() << ", Cmd: " << msgQEntry.m_Cmd.get());
        MsgPtr msgPtr = msgQEntry.m_Msg;
        if (msgQEntry.m_Cmd.get() != nullptr)
        {
            msgQEntry.m_Cmd.get()->ProcessMsg(msgQEntry.m_Msg);
        }
        else
        {
            switch (msgQEntry.m_Msg.get()->GetId())
            {
               case MsgIdServerConstructResponse:
                   DEBUG_TRACE("Server Started");
                   break;

               case MsgIdTCPRecv:
                   OnTCPRecvMsg(msgQEntry.m_Msg);
                   break;

               case MsgIdTCPConnectionException:
                   OnTCPConnectionException(msgQEntry.m_Msg);
                   break;

               default:
                   break;
            }
        }
    }
}


void MasterSchedulerThread::OnTCPConnectionException(MsgPtr msg)
{
    TCPConnectionExceptionMsgPtr pMsg = std::dynamic_pointer_cast<TCPConnectionExceptionMsg>(msg);
    ResourceTracker::Instance().NotifyHostFailure(pMsg->GetConnection()->GetUniqueHostName());
    pMsg->GetConnection()->Close();
}

void MasterSchedulerThread::OnTCPRecvMsg(MsgPtr msg)
{
    DEBUG_TRACE_APPLICATION("On TCP Recv Message");
    TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr.get()->GetId())
    {
       case MsgIdWorkerRegistrationRequest:
           {
               OnWorkerRegistrationRequest(wireMsgPtr);
               break;
           }
       case MsgIdSceneProduceRequest:
           {
               OnSceneProduceRequestMsg(wireMsgPtr);
               break;
           }
       default:
           break;
    }
}

void MasterSchedulerThread::OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr)
{
    WorkerRegistrationMsgPtr registrationMsgPtr = std::dynamic_pointer_cast<WorkerRegistrationMsg>(wireMsgPtr);

    /// Register the host name
    std::string unique_hostname = UniqueServerId(registrationMsgPtr->m_hostname, registrationMsgPtr->m_Port).toString();
    TransportMgr::Instance().SaveConnection(unique_hostname, registrationMsgPtr->GetConnection());
    WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
    reigstrationRespMsgPtr.get()->SetAppTag(registrationMsgPtr->GetAppTag());

    /// We are not expecting any response back. So, pass a nullptr.
    registrationMsgPtr->GetConnection()->SendMsg(reigstrationRespMsgPtr, ListenerPtr(nullptr));

    ResourceTracker::Instance().AddWorker(unique_hostname, registrationMsgPtr->GetNumHwExecutionThread());
}



void MasterSchedulerThread::OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr)
{
    DEBUG_TRACE_APPLICATION("MasterSchedulerThread::OnSceneProduceRequestMsg" << std::hex << this);

    // Check current scheduling policy. Create a command and attach it to this Q.
    CommandPtr cmdPtr(nullptr);
    if (SchedulingPolicyParam::Get().ConfiguredAsStaticScheduler())
    {
        cmdPtr = std::make_shared<SceneSchedulerStatic>(GetListeningQ());
    }
    else
    {
        cmdPtr = std::make_shared<SceneSchedulerDynamic>(GetListeningQ());
    }
    cmdPtr->SaveMemento(cmdPtr);
    cmdPtr->ProcessMsg(wireMsgPtr);
}



Master::Master(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth)
{
    m_num_ray_scheduling_master_threads = num_ray_scheduling_master_threads;
    m_pMasterSchedulerThread = new MasterSchedulerThread *[num_ray_scheduling_master_threads];
    for (int index = 0; index < m_num_ray_scheduling_master_threads; ++index)
    {
        m_pMasterSchedulerThread[index] = new MasterSchedulerThread(scheduling_thread_q_depth);
    }
    m_ThreadPoolLisPtr = std::make_shared<MsgQThreadPoolLis<MasterSchedulerThread> >();
    m_ThreadPoolLisPtr->Construct(m_pMasterSchedulerThread, num_ray_scheduling_master_threads);
}


Master& Master::Instance()
{
    return *g_pMaster;
}

void Master::Start()
{
    for (int index = 0; index < m_num_ray_scheduling_master_threads; ++index)
    {
        // Start the Master Scheduler.
        m_pMasterSchedulerThread[index]->Start();
    }
}

void Master::Instantiate(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth)
{
    g_pMaster = new Master(num_ray_scheduling_master_threads, scheduling_thread_q_depth);
    g_pMaster->Start();
}



