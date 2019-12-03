#include "scene_scheduler_static.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"
#include "resource_tracker.hpp"
#include "master_scheduler.hpp"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

void SceneSchedulerStatic::ProcessMsg(MsgPtr msg)
{
    switch (msg->GetId())
    {
        case MsgIdPixelProduceResponse:
            OnPixelProduceResponseMsg(msg);
            break;

        default:
            SchedulerBase::ProcessMsg(msg);
            break;
    }
}


void SceneSchedulerStatic::KickOffSceneScheduling()
{
    /// Now submit the pixel generation request.
    int currentPixelOffset = 0;
    m_TotalNumPixelsToProduce = m_NX * m_NY;
    m_NumPendingCompletionResponse = 0;

    uint32_t totalAvailableHwThreads = ResourceTracker::Instance().GetTotalNumberOfHwThreads();
    uint32_t numPixelsTobeSchehduled = ResourceTracker::Instance().GetWorkEstimationForNewScene(m_TotalNumPixelsToProduce);
    uint32_t pixelChunkSize = numPixelsTobeSchehduled / totalAvailableHwThreads;
    pixelChunkSize = pixelChunkSize > 0 ? pixelChunkSize : 1;

    /// Prepare pixel chunk indexes.
    std::vector<uint32_t> pixelChunkIndexArray;
    for (int index = 0; index < totalAvailableHwThreads; index++)
    {
        pixelChunkIndexArray.push_back(index);
    }

    if (SchedulingPolicyParam::Get().m_StaticSchedulePolicy == SchedulingPolicyParam::STATIC_SCHEDULE_RANDOM_CHUNK_TO_WORKER)
    {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(pixelChunkIndexArray.begin(), pixelChunkIndexArray.end(), std::default_random_engine(seed));
    }

    int totalScheduled = 0;
    int currentPixelChunkIndex = 0;
    DEBUG_TRACE("m_NX:" << m_NX << ", m_NY:" << m_NY);
    std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
    for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
    {
        TCPIOConnectionPtr p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
        uint32_t numberOfPixelProductionPipelinesForCurrentWorker = workerList[workerIndex]->m_NumAvailablePixelProductionPipelines;

        PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(m_SceneId, numberOfPixelProductionPipelinesForCurrentWorker);
        for (int pixelProductionPipelineId = 0; pixelProductionPipelineId < numberOfPixelProductionPipelinesForCurrentWorker; ++pixelProductionPipelineId)
        {
            if (totalScheduled >= m_TotalNumPixelsToProduce)
            {
                break;
            }
            currentPixelOffset = pixelChunkIndexArray[currentPixelChunkIndex] * pixelChunkSize;

            uint16_t endY  =  Pixel2XYMapper(m_NY, m_NX, currentPixelOffset).Y;
            uint16_t startX = Pixel2XYMapper(m_NY, m_NX, currentPixelOffset).X;

            int workload = pixelChunkSize;

            if (SchedulingPolicyParam::Get().m_StaticSchedulePolicy == SchedulingPolicyParam::STATIC_SCHEDULE_RANDOM_CHUNK_TO_WORKER)
            {
                if (pixelChunkIndexArray[currentPixelChunkIndex] == (pixelChunkIndexArray.size() - 1))
                {
                    workload = m_TotalNumPixelsToProduce - currentPixelOffset;
                }
            }
            else
            {
                if (currentPixelChunkIndex == (pixelChunkIndexArray.size() - 1))
                {
                    workload = m_TotalNumPixelsToProduce - currentPixelOffset;
                }
            }


            uint16_t startY = Pixel2XYMapper(m_NY, m_NX, currentPixelOffset + workload - 1).Y;
            uint16_t endX = Pixel2XYMapper(m_NY, m_NX, currentPixelOffset + workload - 1).X;


            RELEASE_TRACE("Submitting Job to: " << (workerList[workerIndex]->m_UniqueHostName + ":" + std::to_string(pixelProductionPipelineId))
                          << "Job Info: endY:" << endY << ", startY:" << startY << ", startX:" << startX
                          << ", endX:" << endX << "Num Pixels:" << workload);

            /// Update work set
            int appTag = p_connection->AllocateAppTag();
            pixelProduceRequestMsg->Request(pixelProductionPipelineId)->GenerateWork(startY, startX,  endY, endX);
            pixelProduceRequestMsg->Request(pixelProductionPipelineId)->SetPixelDomain(currentPixelOffset, workload);
            pixelProduceRequestMsg->Request(pixelProductionPipelineId)->SetupAppTag(appTag);
            pixelProduceRequestMsg->Request(pixelProductionPipelineId)->SetPipelineId(pixelProductionPipelineId);

            p_connection->RegisterNotification(appTag, m_MyLisPtr);

            /// Track the job
            ResourceTracker::Instance().TrackJob(workerList[workerIndex]->m_UniqueHostName, pixelProductionPipelineId,
                                                 m_SceneId, currentPixelOffset, workload);

            m_NumPendingCompletionResponse++;
            currentPixelChunkIndex++;
            totalScheduled += pixelChunkSize;
        }

        /// Send scene production message. Now we will wait for the response.
        p_connection->SendMsg(pixelProduceRequestMsg, ListenerPtr(nullptr));

        if (totalScheduled >= m_TotalNumPixelsToProduce)
        {
            break;
        }
    }

    //ResourceTracker::Instance().Dump();
}


void SceneSchedulerStatic::OnPixelProduceResponseMsg(MsgPtr msg)
{
    DEBUG_TRACE_APPLICATION("SceneSchedulerStatic::OnPixelProduceResponseMsg:");

    /// Pixel produce response message
    PixelProduceResponseMsgPtr pRespMsg = std::dynamic_pointer_cast<PixelProduceResponseMsg>(msg);

    /// Get the connection from the response message
    TCPIOConnectionPtr pConnection = pRespMsg->GetConnection();


    /// Notify that the job is done
    ResourceTracker::Instance().NotifyJobDone(pConnection->GetUniqueHostName(),
                                              pRespMsg->GetPipelineId(),
                                              m_SceneId,
                                              pRespMsg->GetScenePixelOffset());

    /// Decrement pending responses.
    m_NumPendingCompletionResponse--;

    if (!m_FailedJobs.empty())
    {
        auto failedPixelOffset2Count = m_FailedJobs.begin();
        SendNextFailedJob(pConnection, pRespMsg->GetPipelineId(),
                          failedPixelOffset2Count->first, failedPixelOffset2Count->second);
        m_FailedJobs.erase(failedPixelOffset2Count);
    }
    else
    {

        if (m_NumPendingCompletionResponse == 0)
        {
            ResourceTracker::Instance().Dump();
            RELEASE_TRACE("Scene scheduling has been finished, scene id: " << m_SceneId);
            std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
            for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
            {
                /// Send scene production message. Now we will wait for the response.
                TCPIOConnectionPtr p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
                p_connection->SendMsg(std::make_shared<SceneProduceCleanupMsg>(m_SceneId), ListenerPtr(nullptr));
            }
            // This static scheduling command will now go automatically free itself.
            // We are not closing connection from the client. Once the client closes the connection
            // then we will clean things up.
            m_MyLisPtr = ListenerPtr(nullptr);
        }
    }


    /// Free the Apptag for this connection. All the reference count should get reset now and the command
    /// object should automatically get freed.
    pRespMsg->GetConnection()->FreeAppTag(pRespMsg->GetAppTag());
}


