#include <iostream>
#include "master_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "static_schedule_cmd.hpp"


const int NUM_RAY_SCHEDULER = 32;
const int RAY_SCHEDULER_Q_DEPTH = 128;

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
   WorkerRegistrationMsg *pWireMsg = static_cast<WorkerRegistrationMsg *>(wireMsgPtr.get());
   pWireMsg->Dump();

   /// Register the host name
   std::string unique_hostname = UniqueServerId(pWireMsg->m_hostname, pWireMsg->m_Port).toString();
   TransportMgr::Instance().SaveConnection(unique_hostname, pWireMsg->GetConnection());
   WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
   reigstrationRespMsgPtr.get()->SetAppTag(pWireMsg->GetAppTag());

   /// We are not expecting any response back. So, pass a nullptr.
   pWireMsg->GetConnection()->SendMsg(reigstrationRespMsgPtr, nullptr);

   Master::Instance().AddWorker(unique_hostname);
}



void MasterScheduler::OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr)
{
   DEBUG_TRACE("MasterScheduler::OnSceneProduceRequestMsg" << std::hex << this);
   // Check current scheduling policy. Let's assume we are doing static schedule.
   // Create a command and attach it to this Q.
   StaticScheduleCmdPtr cmdPtr = std::make_shared<StaticScheduleCmd>(Master::Instance().GetWorkerList(), GetListeningQ());
   cmdPtr->ProcessMsg(wireMsgPtr);
}


void Master::AddWorker(std::string &unique_hostname)
{
   std::unique_lock<std::mutex> lck(m_Mutex);
   m_workerlist.push_back(unique_hostname);
   /// Dump the worker lists
   DEBUG_TRACE("worker list: " << m_workerlist.size());
   for (std::vector<std::string>::iterator iter = m_workerlist.begin(); iter != m_workerlist.end(); iter++)
   {
      DEBUG_TRACE("worker: " << *iter);
   }
}

