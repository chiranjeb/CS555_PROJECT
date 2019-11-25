#include "scene_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "resource_tracker.hpp"

void SceneScheduler::ProcessMsg(MsgPtr msg)
{
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


void SceneScheduler::OnSceneProduceRequestMsg(MsgPtr msg)
{
    SceneProduceRequestMsgPtr pRequestMsg = std::dynamic_pointer_cast<SceneProduceRequestMsg>(msg);
    std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();

    m_NX = pRequestMsg->GetNX();
    m_NY = pRequestMsg->GetNY();
    m_SceneId = pRequestMsg->GetSceneId();

    DEBUG_TRACE("sceneDescriptorPtr->GetNY(): " << m_NX << ", sceneDescriptorPtr->GetNX():" << m_NY << ", m_workerList.size()" << workerList.size());
    m_p_client_connection = pRequestMsg->GetConnection();

    int appTag = pRequestMsg->GetAppTag();
    pRequestMsg->SetAppTag(0);

    ///j We need to reserialize the first few bytes....
    pRequestMsg->Repack();

    /// Let's distribute the scene file. This also helps us not doing any serialization/deserialization of the message.
    for (int index = 0; index < workerList.size(); ++index)
    {
        TransportMgr::Instance().FindConnection(workerList[index]->m_UniqueHostName)->SendMsg(pRequestMsg, ListenerPtr(nullptr));
    }

    /// Let's first generate sequential pixel workload
    KickOffSceneScheduling();

    /// all the tasks are scheduled. Send the acknowledgement to the client that the request has been accepted.
    SceneProduceRequestAckMsgPtr sceneProduceRequestAckMsgPtr = std::make_shared<SceneProduceRequestAckMsg>(appTag, STATUS_SUCCESS);

    /// Send the response back to the client.
    pRequestMsg->GetConnection()->SendMsg(sceneProduceRequestAckMsgPtr, ListenerPtr(nullptr));
}



void SceneScheduler::KickOffSceneScheduling()
{
    /// Now submit the pixel generation request.
    m_CurrentPixelOffset = 0;
    m_TotalNumPixelsToProduce = m_NX * m_NY;
    m_NumPendingCompletionResponse = 0;

    uint32_t totalAvailableHwThreads = ResourceTracker::Instance().GetTotalNumberOfHwThreads();
    uint32_t numPixelsTobeSchehduled = ResourceTracker::Instance().GetWorkEstimationForNewScene(m_TotalNumPixelsToProduce);
    uint32_t workloadInPixels = (numPixelsTobeSchehduled + totalAvailableHwThreads - 1) / totalAvailableHwThreads;

    DEBUG_TRACE("m_NX:" << m_NX << ", m_NY:" << m_NY);
    std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
    for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
    {
        TCPIOConnection *p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
        uint32_t numberOfHwExecutionThreadsForCurrentWorker = workerList[workerIndex]->m_NumAvailableHwExecutionThread;

        PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(m_SceneId, numberOfHwExecutionThreadsForCurrentWorker);
        for (int hwExecutionThreadId = 0; hwExecutionThreadId < numberOfHwExecutionThreadsForCurrentWorker; ++hwExecutionThreadId)
        {
            uint16_t endY  =  Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset).Y;
            uint16_t startX = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset).X;

            int workload = workloadInPixels;
            if ((workerIndex == (workerList.size() - 1)) &&
                (hwExecutionThreadId == (numberOfHwExecutionThreadsForCurrentWorker - 1)))
            {
                workload = m_TotalNumPixelsToProduce - m_CurrentPixelOffset;
            }

            uint16_t startY = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset + workload - 1).Y;
            uint16_t endX = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset + workload - 1).X;

            
            RELEASE_TRACE( "Submitting Job to: "<< (workerList[workerIndex]->m_UniqueHostName + ":" + std::to_string(hwExecutionThreadId))  
                           << "Job Info: endY:" << endY << ", startY:" << startY << ", startX:" << startX 
                           << ", endX:" << endX << "Num Pixels:" << workload);

            /// Update work set
            pixelProduceRequestMsg->GenerateWork(hwExecutionThreadId, startY, startX,  endY, endX);
            pixelProduceRequestMsg->SetPixelDomain(hwExecutionThreadId, m_CurrentPixelOffset, workload);

            int appTag = p_connection->AllocateAppTag();
            pixelProduceRequestMsg->SetupAppTag(hwExecutionThreadId, appTag);
            p_connection->RegisterNotification(appTag, m_MyLisPtr);

            /// Track the job
            ResourceTracker::Instance().TrackJob(workerList[workerIndex]->m_UniqueHostName, hwExecutionThreadId,
                                                 m_SceneId, m_CurrentPixelOffset, workload);

            /// Update the pixel offset
            m_CurrentPixelOffset += workloadInPixels;
            m_NumPendingCompletionResponse++;
        }

        /// Send scene production message. Now we will wait for the response.
        p_connection->SendMsg(pixelProduceRequestMsg, ListenerPtr(nullptr));
    }

    //ResourceTracker::Instance().Dump();
}


void SceneScheduler::OnPixelProduceResponseMsg(MsgPtr msg)
{
    DEBUG_TRACE_APPLICATION("SceneScheduler::OnPixelProduceResponseMsg:");

    /// Pixel produce response message
    PixelProduceResponseMsgPtr pRespMsg = std::dynamic_pointer_cast<PixelProduceResponseMsg>(msg);

    /// Get the connection from the response message
    TCPIOConnection *pConnection = pRespMsg->GetConnection();


    /// Notify that the job is done
    ResourceTracker::Instance().NotifyJobDone(pConnection->GetRemoteHostName(),
                                              pRespMsg->GetThreadId(),
                                              m_SceneId,
                                              pRespMsg->GetScenePixelOffset());

    /// Decrement pending responses.
    m_NumPendingCompletionResponse--;

    if (m_NumPendingCompletionResponse == 0)
    {
        ResourceTracker::Instance().Dump();
        RELEASE_TRACE("Scene scheduling has been finished, scene id: " << m_SceneId);
        std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
        for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
        {
            /// Send scene production message. Now we will wait for the response.
            TCPIOConnection *p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
            p_connection->SendMsg(std::make_shared<SceneProduceCleanupMsg>(m_SceneId), ListenerPtr(nullptr));
        }
        // This static scheduling command will now go automatically free itself.
        // We are not closing connection from the client. Once the client closes the connection
        // then we will clean things up.
        // Close the connection.
        // m_p_client_connection->Close();
        m_MyLisPtr = ListenerPtr(nullptr);
    }


    /// Free the Apptag for this connection. All the reference count should get reset now and the command
    /// object should automatically get freed.
    pRespMsg->GetConnection()->FreeAppTag(pRespMsg->GetAppTag());
}


