#include <iostream>
#include "client.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include <functional>



// file stream pointer
std::ofstream m_file_output_stream;

void Client::SetupSceneName(std::string scene_name)
{
   m_scene_name = scene_name;
}

/// Setup master info
void Client::SetupMasterInfo(std::string master_address, int master_port)
{
   m_master_address = master_address;
   m_master_port = master_port;
}

void Client::Run()
{
   RELEASE_TRACE("Started Client thread");
   while (1)
   {
      MsgQEntry msgQEntry = TakeNext();
      MsgPtr msgPtr = msgQEntry.m_Msg;
      DEBUG_TRACE("Client ProcessMsg: " << std::hex << msgQEntry.m_Msg.get()->GetId());
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
            case MsgIdSceneProduceRequestAck:
               {
                  OnSceneProduceRequestAckMsg(msgQEntry.m_Msg);
                  break;
               }
            case MsgIdSceneSegmentProduceResponse:
               {
                  OnSceneSegmentProduceRespMsg(msgQEntry.m_Msg);
                  break;
               }

            case MsgIdTCPRecv:
               {
                  OnTCPRecvMsg(msgQEntry.m_Msg);
                  break;
               }

            default:
               break;
         }
      }
   }
}

void Client::OnTCPRecvMsg(MsgPtr msg)
{
   DEBUG_TRACE("On TCP Recv Message(this:" << std::hex << this << ")");
   TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
   WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

   switch (wireMsgPtr.get()->GetId())
   {
      case MsgIdSceneSegmentProduceResponse:
         {
            OnSceneSegmentProduceRespMsg(wireMsgPtr);
            break;
         }
      default:
         break;
   }
}

void Client::Start()
{
   m_thread = new std::thread(&Client::Run, *this);
}

void Client::OnCreateServerResponse(MsgPtr msg)
{
   DEBUG_TRACE("Worker::OnCreateServerResponse");
   TCPServerConstructStatusMsg *p_responseMsg =  static_cast<TCPServerConstructStatusMsg *>(msg.get());
   m_listening_port  = p_responseMsg->GetPort();

   // Let's establish a connection
   TransportMgr::Instance().EstablishNewConnection(m_master_address, m_master_port, GetThrdListener(), true);
}

void Client::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
   RELEASE_TRACE("Successfully established connection");
   TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
   m_p_ConnectionToMaster = p_responseMsg->GetConnection();
   SceneDescriptorPtr sceneDescriptorPtr = SceneFactory::GetScene(m_scene_name);

   // Create the output file pointer.
   m_file_output_stream.open(m_scene_name + ".ppm");


   m_SceneSizeInPixels = sceneDescriptorPtr->GetNX() * sceneDescriptorPtr->GetNY();

   /// write header
   m_file_output_stream << "P3\n" << sceneDescriptorPtr->GetNX() << " " << sceneDescriptorPtr->GetNY() << "\n255\n";

   // Create the scene produce request message.
   SceneProduceRequestMsgPtr requestMsg = std::make_shared<SceneProduceRequestMsg>(sceneDescriptorPtr);

   time_t _tm = time(NULL);
   struct tm *curtime = localtime(&_tm);
   std::size_t scene_id = std::hash<std::string>
   {}
   (m_scene_name + asctime(curtime));

   requestMsg->SetSceneId(scene_id);

   DEBUG_TRACE("sceneDescriptorPtr->GetNY():" << sceneDescriptorPtr->GetNY() << "sceneDescriptorPtr->GetNX():" << sceneDescriptorPtr->GetNX());
   requestMsg->SetImageDimension(sceneDescriptorPtr->GetNX(), sceneDescriptorPtr->GetNY());
   requestMsg->SetAnswerBackAddress(TransportMgr::Instance().MyName(), m_listening_port);
   requestMsg->SetAppTag(m_p_ConnectionToMaster->AllocateAppTag());
   m_p_ConnectionToMaster->SendMsg(requestMsg, GetThrdListener());
}

void Client::OnSceneProduceRequestAckMsg(MsgPtr msg)
{
   RELEASE_TRACE("Client::OnSceneProduceRequestAckMsg");
   WireMsgPtr respMsgPtr = std::dynamic_pointer_cast<WireMsg>(msg);
   respMsgPtr->GetConnection()->FreeAppTag(respMsgPtr->GetAppTag());
}

void Client::OnSceneSegmentProduceRespMsg(MsgPtr msg)
{
   SceneSegmentProduceResponseMsgPtr respMsgPtr = std::dynamic_pointer_cast<SceneSegmentProduceResponseMsg>(msg);
   RELEASE_TRACE("Client::Received a scene produce response message, respMsgPtr->GetScenePixelOffset():" << respMsgPtr->GetScenePixelOffset());

   if (m_CurrentPixelToWrite == respMsgPtr->GetScenePixelOffset())
   {
      RELEASE_TRACE("m_CurrentPixelToWrite:" << m_CurrentPixelToWrite << "respMsgPtr->GetScenePixelOffset():" << respMsgPtr->GetScenePixelOffset());

      // write out the file.
      std::pair<uint8_t *, uint32_t> sceneSegmentBuffer = respMsgPtr->GetSceneBuffer();
      m_file_output_stream.write(reinterpret_cast<char *>(sceneSegmentBuffer.first), sceneSegmentBuffer.second);
      m_CurrentPixelToWrite += respMsgPtr->GetNumPixels();

      // The pixels are produced in the final format which contains space and end lines. We can remove them and transfer over
      // the network to reduce network I/O. For time being, let's just stich using the final format.
      for (std::set<MsgPtr>::iterator iter = m_SceneSegmentResponseSet.begin(); iter != m_SceneSegmentResponseSet.end();)
      {
         SceneSegmentProduceResponseMsgPtr segmentMsgPtr = std::dynamic_pointer_cast<SceneSegmentProduceResponseMsg>((*iter));
         if (segmentMsgPtr->GetScenePixelOffset() == m_CurrentPixelToWrite)
         {
            sceneSegmentBuffer = segmentMsgPtr->GetSceneBuffer();
            m_file_output_stream.write(reinterpret_cast<char *>(sceneSegmentBuffer.first), sceneSegmentBuffer.second);
            m_CurrentPixelToWrite += segmentMsgPtr->GetNumPixels();

            iter = m_SceneSegmentResponseSet.erase(iter);
         }
         else
         {
            iter++;
         }
      }

      if (m_CurrentPixelToWrite == m_SceneSizeInPixels)
      {
         // We are done. close connections and exit.
         RELEASE_TRACE("Successfully produced the image.");
         m_file_output_stream.close();
         exit(0);
      }
   }
   else
   {
      // We can get out of order response.. So, we need to handle this carefully. Keep it in sorted
      // based on the offset and release it as soon as we have enough sequential stuff.
      m_SceneSegmentResponseSet.insert(respMsgPtr);
   }
}
