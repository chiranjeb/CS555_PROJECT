#include "static_schedule_cmd.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "resource_tracker.hpp"

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
    std::vector<ResourceEntryPtr> &workerList = ResourceTracker::Instance().GetWorkers();

    m_NX = pRequestMsg->GetNX();
    m_NY = pRequestMsg->GetNY();
    DEBUG_TRACE("sceneDescriptorPtr->GetNY(): " << m_NX << ", sceneDescriptorPtr->GetNX():" << m_NY << ", m_workerList.size()" << workerList.size());
    m_p_client_connection = pRequestMsg->GetConnection();

    int appTag = pRequestMsg->GetAppTag();
    pRequestMsg->SetAppTag(0);

    ///j We need to reserialize the first few bytes....
    pRequestMsg->Repack();

    /// Let's distribute the scene file. This also helps us not doing any serialization/deserialization of the message.
    for (int index = 0; index < workerList.size(); ++index)
    {
        TransportMgr::Instance().FindConnection(workerList[index]->m_unique_host_name)->SendMsg(pRequestMsg, nullptr);
    }

    /// Let's first generate sequential pixel workload
    GenerateSequentialPixelWorkload(pRequestMsg->GetSceneId());

    /// all the tasks are scheduled. Send the acknowledgement to the client that the request has been accepted.
    SceneProduceRequestAckMsgPtr sceneProduceRequestAckMsgPtr = std::make_shared<SceneProduceRequestAckMsg>(appTag, STATUS_SUCCESS);

    /// Send the response back to the client.
    pRequestMsg->GetConnection()->SendMsg(sceneProduceRequestAckMsgPtr, nullptr);
}



void StaticScheduleCmd::GenerateSequentialPixelWorkload(std::size_t sceneId)
{
    /// Now submit the pixel generation request.
    int pixelOffset = 0;
    int totalPixels = m_NX * m_NY ;
    m_NumPendingCompletionResponse = 0;
    uint32_t totalAvailableHwThreads = ResourceTracker::Instance().GetTotalNumberOfHwThreads();
    uint32_t workloadInPixels = (totalPixels + totalAvailableHwThreads -1)/totalAvailableHwThreads;

    DEBUG_TRACE("m_NX:" << m_NX << "m_NY:" << m_NY);
    std::vector<ResourceEntryPtr> &workerList = ResourceTracker::Instance().GetWorkers();
    for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex )
    {
       uint32_t numberOfHwExecutionThreadsForCurrentWorker = workerList[workerIndex]->m_available_hw_execution_thread;
       PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(sceneId, numberOfHwExecutionThreadsForCurrentWorker);
       for (int hwExecutionThreadId = 0; hwExecutionThreadId < numberOfHwExecutionThreadsForCurrentWorker; ++hwExecutionThreadId)
       {
             uint16_t endY  =  Pixel2XYMapper(m_NY, m_NX, pixelOffset).Y ;
             uint16_t startX = Pixel2XYMapper(m_NY, m_NX, pixelOffset).X ;

             int workload = workloadInPixels;
             if ((workerIndex == (workerList.size()-1)) && 
                 (hwExecutionThreadId == (numberOfHwExecutionThreadsForCurrentWorker-1)))
             {
                workload = totalPixels - pixelOffset;
             }

             uint16_t startY = Pixel2XYMapper(m_NY, m_NX, pixelOffset+workload-1).Y ;
             uint16_t endX = Pixel2XYMapper(m_NY, m_NX, pixelOffset+workload-1).X ;
             DEBUG_TRACE("endY:" << endY << ", startY:" << startY << ", startX:" << startX << ", endX:" << endX << std::endl);
             /// Update work set
             pixelProduceRequestMsg->GenerateWork(hwExecutionThreadId, startY, startX,  endY, endX);
             pixelProduceRequestMsg->SetPixelDomain(hwExecutionThreadId, pixelOffset, workload);
             pixelOffset += workloadInPixels;
       }

       /// Send scene production message. Now we will wait for the response.
       TCPIOConnection *p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_unique_host_name);
       pixelProduceRequestMsg->SetAppTag(p_connection->AllocateAppTag());
       p_connection->SendMsg(pixelProduceRequestMsg, this);
       m_NumPendingCompletionResponse++;
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


