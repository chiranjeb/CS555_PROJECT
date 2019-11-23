#include <iostream>
#include "worker.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include "wiremsg/pixel_produce_msg.hpp"

#define SCENE_PRODUCER_Q_DEPTH 32
#define NUM_SCENE_PRODUCER 16

BlockingQueue<MsgQEntry> g_SceneProducerQ(SCENE_PRODUCER_Q_DEPTH);
ThreadPoolManager m_ThreadPoolMgr(NUM_SCENE_PRODUCER, std::make_shared<BlockingQueue<MsgQEntry>>(SCENE_PRODUCER_Q_DEPTH));

void Worker::Run()
{
   RELEASE_TRACE("Started Worker thread");
   while (1)
   {
      MsgQEntry msgQEntry = TakeNext();
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
               {
                  OnCreateServerResponse(msgQEntry.m_Msg);
                  break;
               }
            case MsgIdConnectionEstablishmentResponse:
               {
                  OnConnectionEstablishmentResponseMsg(msgQEntry.m_Msg);
                  break;
               }
            case MsgIdWorkerRegistrationResponse:
               {
                  OnWorkerRegistrationRespMsg(msgQEntry.m_Msg);
                  break;
               }

            case MsgIdTCPRecv:
               OnTCPRecvMsg(msgQEntry.m_Msg);
               break;


            default:
               break;
         }
      }
   }
}

void Worker::OnTCPRecvMsg(MsgPtr msg)
{
   DEBUG_TRACE("On TCP Recv Message");
   TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
   WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

   switch (wireMsgPtr.get()->GetId())
   {
      case MsgIdSceneProduceRequest:
         {
            OnSceneProduceRequestMsg(msg);
            break;
         }

      case MsgIdPixelProduceRequest:
         {
            OnPixelProduceRequestMsg(msg);
            break;
         }
      default:
         break;
   }
}

void Worker::OnCreateServerResponse(MsgPtr msg)
{
   DEBUG_TRACE("Worker::OnCreateServerResponse");
   TCPServerConstructStatusMsg *p_responseMsg =  static_cast<TCPServerConstructStatusMsg *>(msg.get());
   m_listening_port  = p_responseMsg->GetPort();

   // Let's establish a connection
   m_p_ConnectionToMaster = nullptr;
   TransportMgr::Instance().EstablishNewConnection(m_master_address, m_master_port, GetThrdListener(), true);
}

void Worker::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
   RELEASE_TRACE("Successfully established connection");
   TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
   if (m_p_ConnectionToMaster == nullptr)
   {
      m_p_ConnectionToMaster = p_responseMsg->GetConnection();
      std::string hostname = TransportMgr::Instance().MyName();
      WorkerRegistrationMsgPtr reigstrationMsgPtr =  WorkerRegistrationMsgPtr(new WorkerRegistrationMsg(hostname, m_listening_port));
      /// We are expecting response, So allocate an apptag and pass our thread listener which will route back the message to us.
      reigstrationMsgPtr.get()->SetAppTag(m_p_ConnectionToMaster->AllocateAppTag());
      m_p_ConnectionToMaster->SendMsg(reigstrationMsgPtr, GetThrdListener());
   }
   else
   {
      UniqueServerId serverId(p_responseMsg->GetServerAddress(), p_responseMsg->GetServerPort());
      std::size_t sceneId = m_Client2SceneId[serverId.toString()];
      m_SceneId2Connection.insert(std::pair<std::size_t, TCPIOConnection *>(sceneId, p_responseMsg->GetConnection()));
      m_WaitersForConnectionSetup[sceneId]->SetConnection(p_responseMsg->GetConnection());
      m_WaitersForConnectionSetup.erase(sceneId);
   }
}

void Worker::OnWorkerRegistrationRespMsg(MsgPtr msg)
{
   RELEASE_TRACE("Worker::OnWorkerRegistrationRespMsg");
   WorkerRegistrationRespMsg *p_responseMsg =  static_cast<WorkerRegistrationRespMsg *>(msg.get());
   p_responseMsg->Dump();
   p_responseMsg->GetConnection()->FreeAppTag(p_responseMsg->GetAppTag());
}

void Worker::OnSceneProduceRequestMsg(MsgPtr msg)
{
   /// save the scene id.
   SceneProduceRequestMsgPtr requestMsgPtr  = std::dynamic_pointer_cast<SceneProduceRequestMsg>(msg);

   /// Update scene file map
   m_SceneFileMap.insert(std::pair<std::size_t, SceneProduceRequestMsgPtr>(requestMsgPtr->GetSceneId(), requestMsgPtr));

   /// Scene to client for forward look up
   UniqueServerId serverId = UniqueServerId(requestMsgPtr->GetClientAddress(), requestMsgPtr->GetClientPort());
   m_SceneId2Client.insert(std::pair<std::size_t, std::string>(requestMsgPtr->GetSceneId(), serverId.toString()));

   /// Client to scene look up
   m_Client2SceneId.insert(std::pair<std::string, std::size_t>(serverId.toString(), requestMsgPtr->GetSceneId()));

   /// Establishes a connection with the client
   TransportMgr::Instance().EstablishNewConnection(requestMsgPtr->GetClientAddress(), requestMsgPtr->GetClientPort(), GetThrdListener(), true);
}

void Worker::OnSceneProduceDone(MsgPtr msg)
{
   /*
   DEBUG_TRACE("Worker::OnPixelProduceRequestMsg: ");
   SceneProduceCleanupMsgPtr requestMsgPtr  = dynamic_pointer_cast<SceneProduceCleanupMsg>(msg);

   m_SceneFileMap.erase(requestMsgPtr->GetSceneId());

   /// Client to scene look up
   m_Client2SceneId.erase(m_SceneId2Client[requestMsgPtr->GetSceneId()]);

   /// Scene to client for forward look up
   m_SceneId2Client.erase(requestMsgPtr->GetSceneId());

   /// Remove scene Id to connection
   m_SceneId2Connection.erase(requestMsgPtr->GetSceneId());*/
}

void Worker::OnPixelProduceRequestMsg(MsgPtr msg)
{
   /// Now we will start producing pixels for which we have already established all the contexts.
   PixelProduceRequestMsgPtr requestMsgPtr  = std::dynamic_pointer_cast<PixelProduceRequestMsg>(msg);
   TCPIOConnection *pConnection = m_SceneId2Connection[requestMsgPtr->GetSceneId()];
   PixelProducerPtr cmdPtr = std::make_shared<PixelProducer>(m_ThreadPoolMgr.GetListeningQ(), pConnection);
   m_ThreadPoolMgr.Send(MsgQEntry(requestMsgPtr, cmdPtr));
   if (pConnection == nullptr)
   {
      m_WaitersForConnectionSetup.insert(std::pair<std::size_t, PixelProducerPtr>(requestMsgPtr->GetSceneId(), cmdPtr));
   }
}




