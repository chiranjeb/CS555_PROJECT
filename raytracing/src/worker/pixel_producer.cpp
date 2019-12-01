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

void PixelProducer::OnPixelProduceRequestMsg(MsgPtr msg)
{
    RELEASE_TRACE("Start Pixel Production( Obj:" << std::hex << this << ")");
    PixelProduceRequestMsgPtr pRequestMsg = std::dynamic_pointer_cast<PixelProduceRequestMsg>(msg);

    SceneSegmentProduceResponseMsgPtr respMsgPtr = std::make_shared<SceneSegmentProduceResponseMsg>(pRequestMsg->GetSceneId(),
                                                                                                    pRequestMsg->GetNumPixels(m_requestIndex),
                                                                                                    pRequestMsg->GetScenePixelOffset(m_requestIndex));

    /// Stream through so that we don't have to worry about serializing it later.
    PreAllocatedStreamBuffer streambuff(reinterpret_cast<char *>(respMsgPtr->GetPixelBufferStart()), respMsgPtr->GetPixelBufferMaxLimit());
    std::ostream ostrm(&streambuff);


    DEBUG_TRACE_APPLICATION("Worker::OnPixelProduceRequestMsg: GetScene Descriptor:" << streambuff.Tellp());
    SceneDescriptorPtr sceneDescriptorPtr = Worker::Instance().GetSceneDescriptor(pRequestMsg->GetSceneId());

    /// Produce pixels
    PixelProduceRequest *pRequest = pRequestMsg->GetRequest(m_requestIndex);
    ProducePixels(pRequest->m_endY, pRequest->m_startY, pRequest->m_endX, pRequest->m_startX, pRequest->m_RPP,
                  sceneDescriptorPtr, ostrm);

    /// Update valid buffer
    respMsgPtr->UpdateValidBuffer(streambuff.Tellp());


    RELEASE_TRACE("Pixel Production( Obj: " << std::hex << this << ") Done....Produced Buffer size: " << streambuff.Tellp());

    /// Pack partial stuff.
    respMsgPtr->PackPartial();
    if (m_p_clientConnection != nullptr)
    {
        /// Ship the result to client. We are not waiting for the response.
        m_p_clientConnection->SendMsg(respMsgPtr, ListenerPtr(nullptr));
    }
    else
    {
        // This command will be kicked out when connection is made
    }

    // Send the response to the master scheduler.
    PixelProduceResponseMsgPtr respMsg = std::make_shared<PixelProduceResponseMsg>(pRequestMsg->GetSceneId(),
                                                                                   pRequestMsg->GetNumPixels(m_requestIndex),
                                                                                   pRequestMsg->GetScenePixelOffset(m_requestIndex),
                                                                                   pRequestMsg->GetRequest(m_requestIndex)->m_ThreadId);
    respMsg->SetAppTag(pRequestMsg->GetRequest(m_requestIndex)->m_AppTag);
    m_pConnectionToMaster->SendMsg(respMsg, ListenerPtr(nullptr));
}


