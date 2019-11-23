#include <iostream>
#include "pixel_producer.hpp"
#include "worker.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "transport/transport_mgr.hpp"
#include "ray_tracer/scene_descriptor.hpp"

void PixelProducer::ProcessMsg(MsgPtr msg)
{
   RELEASE_TRACE("PixelProducer::ReceivedMsg: " << msg->GetId());
   while (1)
   {
      switch (msg->GetId())
      {
         case MsgIdPixelProduceRequest:
            {
               OnPixelProduceRequestMsg(msg);
               break;
            }
         default:
            break;
      }
   }
}

void PixelProducer::OnPixelProduceRequestMsg(MsgPtr msg)
{
   DEBUG_TRACE("Worker::OnPixelProduceRequestMsg: ");
   PixelProduceRequestMsgPtr pRequestMsg = std::dynamic_pointer_cast<PixelProduceRequestMsg>(msg);
   SceneSegmentProduceResponseMsgPtr respMsgPtr = std::make_shared<SceneSegmentProduceResponseMsg>(pRequestMsg->GetSceneId(),
                                                                                                   pRequestMsg->GetNumPixels(),
                                                                                                   pRequestMsg->GetScenePixelOffset());
   // Stream through so that we don't have to worry about serializing later one.
   PreAllocatedStreamBuffer streambuff(reinterpret_cast<char *>(respMsgPtr->GetPixelBufferStart()), respMsgPtr->GetPixelBufferMaxLimit());
   std::ostream ostrm(&streambuff);

   SceneDescriptorPtr sceneDescriptorPtr = Worker::Instance().GetSceneDescriptor(pRequestMsg->GetSceneId());

   ProducePixels(pRequestMsg->m_endY, pRequestMsg->m_startY, pRequestMsg->m_endX, pRequestMsg->m_startX,
                 sceneDescriptorPtr, ostrm);

   if (m_p_clientConnection != nullptr)
   {
      // Ship the result to the client first
      m_p_clientConnection->SendMsg(respMsgPtr, nullptr);
   }
   else
   {
      // This command will be kicked out when connection is made
   }

   // Send the response to the master scheduler.
   PixelProduceResponseMsgPtr respMsg = std::make_shared<PixelProduceResponseMsg>(pRequestMsg->GetSceneId());
   respMsg->SetAppTag(pRequestMsg->GetAppTag());
   Worker::Instance().GetConnectionToMaster()->SendMsg(respMsg, nullptr);
}



void PixelProducer::OnSceneProduceDone(MsgPtr msg)
{
}




