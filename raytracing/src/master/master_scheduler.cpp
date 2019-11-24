#include <iostream>
#include "master_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "static_schedule_cmd.hpp"

namespace
{
Master *g_pMaster;
SchedulingPolicyParam g_SchedulingPolicyParam;
}


SchedulingPolicyParam& SchedulingPolicyParam::Get()
{
   return g_SchedulingPolicyParam;
}

MasterScheduler::MasterScheduler(int scheduling_thread_q_depth) : MsgQThread("MasterScheduler", scheduling_thread_q_depth)
{
}

void MasterScheduler::Start()
{
   m_thread = new std::thread(&MasterScheduler::Run, *this);
}

void MasterScheduler::Run()
{
   while (1)
   {
      MsgQEntry msgQEntry = TakeNext();
      DEBUG_TRACE("Got a message:" << std::hex << this);
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

            default:
               break;
         }
      }
   }
}

void MasterScheduler::OnTCPRecvMsg(MsgPtr msg)
{
   DEBUG_TRACE("On TCP Recv Message");
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

void MasterScheduler::OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr)
{
   WorkerRegistrationMsgPtr registrationMsgPtr = std::dynamic_pointer_cast<WorkerRegistrationMsg>(wireMsgPtr);

   /// Register the host name
   std::string unique_hostname = UniqueServerId(registrationMsgPtr->m_hostname, registrationMsgPtr->m_Port).toString();
   TransportMgr::Instance().SaveConnection(unique_hostname, registrationMsgPtr->GetConnection());
   WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
   reigstrationRespMsgPtr.get()->SetAppTag(registrationMsgPtr->GetAppTag());

   /// We are not expecting any response back. So, pass a nullptr.
   registrationMsgPtr->GetConnection()->SendMsg(reigstrationRespMsgPtr, nullptr);

   ResourceTracker::Instance().AddWorker(unique_hostname, registrationMsgPtr->GetNumHwExecutionThread());
}



void MasterScheduler::OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr)
{
   DEBUG_TRACE("MasterScheduler::OnSceneProduceRequestMsg" << std::hex << this);

   // Check current scheduling policy. Create a command and attach it to this Q.
   if (SchedulingPolicyParam::Get().ConfiguredAsStaticScheduler())
   {
      StaticScheduleCmdPtr cmdPtr = std::make_shared<StaticScheduleCmd>(GetListeningQ());
      cmdPtr->ProcessMsg(wireMsgPtr);
   }
   else
   {
      /// dynamic scheduling code goes here
   }
}



Master::Master(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth)
{
   m_num_ray_scheduling_master_threads = num_ray_scheduling_master_threads;
   m_pMasterScheduler = new MasterScheduler* [num_ray_scheduling_master_threads];
   for (int index = 0; index < m_num_ray_scheduling_master_threads; ++index)
   {
      m_pMasterScheduler[index] = new MasterScheduler(scheduling_thread_q_depth);
   }
   m_ThreadPoolLis.Construct(m_pMasterScheduler, num_ray_scheduling_master_threads);
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
      m_pMasterScheduler[index]->Start();
   }
}

void Master::Instantiate(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth, int scheduling_policy)
{
   g_pMaster = new Master(num_ray_scheduling_master_threads, scheduling_thread_q_depth);
   g_SchedulingPolicyParam.m_Policy = scheduling_policy;
   g_pMaster->Start();
}



