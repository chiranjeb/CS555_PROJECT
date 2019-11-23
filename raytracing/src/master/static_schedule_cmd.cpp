#include "static_schedule_cmd.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"

void StaticScheduleCmd::ProcessMsg(MsgPtr msg)
{
    DEBUG_TRACE("StaticScheduleCmd::ProcessMsg - Received MsgId: " << std::hex << msg->GetId());
    switch (msg->GetId())
    {
        case MsgIdSceneProduceRequest:
            OnSceneProduceRequestMsg(msg);
            break;

        case MsgIdPixelProduceResponse:
            OnPixelProduceResponseMsg(msg);
            break;

        default:
            break;
    }
}


void StaticScheduleCmd::OnSceneProduceRequestMsg(MsgPtr msg)
{
    SceneProduceRequestMsgPtr pRequestMsg = std::dynamic_pointer_cast<SceneProduceRequestMsg>(msg);

    m_NX = pRequestMsg->GetNX();
    m_NY = pRequestMsg->GetNY();
    DEBUG_TRACE("sceneDescriptorPtr->GetNY(): " << m_NX << ", sceneDescriptorPtr->GetNX():" << m_NY << ", m_workerList.size()" << m_workerList.size());
    m_workloadInPixels = ((m_NX * m_NY + m_workerList.size() - 1)/ m_workerList.size());
    m_p_client_connection = pRequestMsg->GetConnection();

    int appTag = pRequestMsg->GetAppTag();
    pRequestMsg->SetAppTag(0);

    ///j We need to reserialize the first few bytes....
    pRequestMsg->Repack();

    /// Let's distribute the scene file. This also helps us not doing any serialization/deserialization of the message.
    for (int index = 0; index < m_workerList.size(); ++index)
    {
        TransportMgr::Instance().FindConnection(m_workerList[index])->SendMsg(pRequestMsg, nullptr);
    }

    /// Let's first generate sequential pixel workload
    GenerateSequentialPixelWorkload(pRequestMsg->GetSceneId());

    /// all the tasks are scheduled. Send the acknowledgement to the client that the request has been accepted.
    SceneProduceRequestAckMsgPtr sceneProduceRequestAckMsgPtr = std::make_shared<SceneProduceRequestAckMsg>(appTag, STATUS_SUCCESS);

    /// Send the response back to the client.
    pRequestMsg->GetConnection()->SendMsg(sceneProduceRequestAckMsgPtr, nullptr);
}

struct Pixel2XYMapper
{
   Pixel2XYMapper(int Ny, int Nx, int pixelPos)
   {
      Y = Ny - ((pixelPos/ Nx) + 1) ;
      X = pixelPos % Nx;
   }
   int X;
   int Y;
};


void StaticScheduleCmd::GenerateSequentialPixelWorkload(std::size_t sceneId)
{
    /// Now submit the pixel generation request.
    int pixelOffset = 0;
    int totalPixels = m_NX * m_NY ;
    DEBUG_TRACE("sceneDescriptorPtr->GetNY():" << m_NX << "sceneDescriptorPtr->GetNX():" << m_NY);
    for (int index = 0; index < m_workerList.size(); ++index)
    {
        DEBUG_TRACE("Send request to worker index:" << index << ", pixelOffset:" << pixelOffset << ", m_workloadInPixels:" << m_workloadInPixels);
        /// Create a scene description message for each  worker. Include some work in the scene description too.
        PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(sceneId);

        int workload = m_workloadInPixels;
        if (index == (m_workerList.size()-1))
        {
           workload = totalPixels - pixelOffset;
        }

        int endY  =  Pixel2XYMapper(m_NY, m_NX, pixelOffset).Y ;
        int startX = Pixel2XYMapper(m_NY, m_NX, pixelOffset).X ;

        int startY = Pixel2XYMapper(m_NY, m_NX, pixelOffset+workload-1).Y ;
        int endX = Pixel2XYMapper(m_NY, m_NX, pixelOffset+workload-1).X ;


        DEBUG_TRACE("endY:" << endY << ", startY:" << startY << ", startX:" << startX << ", endX:" << endX << std::endl);
        /// Update work set
        pixelProduceRequestMsg->GenerateWork(startY, startX,  endY, endX);
        pixelProduceRequestMsg->SetPixelDomain(pixelOffset, workload);

        /// Update the scene id.
        m_NumPendingCompletionResponse = 0;

        /// Send scene production message. Now we will wait for the response.
        TCPIOConnection *p_connection = TransportMgr::Instance().FindConnection(m_workerList[index]);
        pixelProduceRequestMsg->SetAppTag(p_connection->AllocateAppTag());
        p_connection->SendMsg(pixelProduceRequestMsg, this);
        pixelOffset += m_workloadInPixels;
    }
}


void StaticScheduleCmd::OnPixelProduceResponseMsg(MsgPtr msg)
{
    PixelProduceResponseMsgPtr pRespMsg = std::dynamic_pointer_cast<PixelProduceResponseMsg>(msg);
    pRespMsg->GetConnection()->FreeAppTag(pRespMsg->GetAppTag());
    m_NumPendingCompletionResponse--;

    if (m_NumPendingCompletionResponse == 0)
    {
        DEBUG_TRACE("Work has been finished.");
        // This static scheduling command will now go automatically free itself.
        // We are not closing connection from the client. Once the client closes the connection
        // then we will clean things up.
        // Close the connection.
        // m_p_client_connection->Close();
    }
}


