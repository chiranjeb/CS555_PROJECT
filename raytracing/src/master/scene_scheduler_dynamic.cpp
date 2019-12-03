#include "scene_scheduler_dynamic.hpp"
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

void SceneSchedulerDynamic::ProcessMsg(MsgPtr msg)
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





void SceneSchedulerDynamic::KickOffSceneScheduling()
{
    /// Now submit the pixel generation request.
    m_CurrentPixelOffset = 0;
    m_TotalNumPixelsToProduce = m_NX * m_NY;
    m_NumPendingCompletionResponse = 0;

    uint32_t totalAvailableHwThreads = ResourceTracker::Instance().GetTotalNumberOfHwThreads();
    uint32_t numPixelsTobeSchehduled = ResourceTracker::Instance().GetWorkEstimationForNewScene(m_TotalNumPixelsToProduce);

    if (numPixelsTobeSchehduled < totalAvailableHwThreads)
    {
        m_workload = 1;
    }
    else
    {
        m_workload = numPixelsTobeSchehduled / totalAvailableHwThreads;
    }

    /// Prepare pixel chunk indexes.
    int currentPixelChunkIndex = 0;
    DEBUG_TRACE("m_NX:" << m_NX << ", m_NY:" << m_NY);
    std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
    for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
    {
        TCPIOConnectionPtr p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
        uint32_t numberOfHwExecutionThreadsForCurrentWorker = workerList[workerIndex]->m_NumAvailableHwExecutionThread;
        SendNextJob(p_connection, 0, numberOfHwExecutionThreadsForCurrentWorker);
    }
    for (int workerIndex = 0; workerIndex < workerList.size(); ++workerIndex)
    {
        TCPIOConnectionPtr p_connection = TransportMgr::Instance().FindConnection(workerList[workerIndex]->m_UniqueHostName);
        uint32_t numberOfHwExecutionThreadsForCurrentWorker = workerList[workerIndex]->m_NumAvailableHwExecutionThread;
        SendNextJob(p_connection, 0, numberOfHwExecutionThreadsForCurrentWorker);
    }
    //ResourceTracker::Instance().Dump();
}


void SceneSchedulerDynamic::OnPixelProduceResponseMsg(MsgPtr msg)
{
    DEBUG_TRACE_APPLICATION("SceneSchedulerDynamic::OnPixelProduceResponseMsg:");

    /// Pixel produce response message
    PixelProduceResponseMsgPtr pRespMsg = std::dynamic_pointer_cast<PixelProduceResponseMsg>(msg);

    /// Get the connection from the response message
    TCPIOConnectionPtr pConnection = pRespMsg->GetConnection();

    /// Notify that the job is done
    ResourceTracker::Instance().NotifyJobDone(pConnection->GetUniqueHostName(),
                                              pRespMsg->GetThreadId(),
                                              m_SceneId,
                                              pRespMsg->GetScenePixelOffset());

    if (pRespMsg->GetNumPixels() != 0)
    {
        /// To simply the underrun condition on the workload which is way smaller than
        /// the cluster's capcity, we do send some messages without work....Just to
        /// go through the normal path. However, we don't track those jobs which is tied to
        /// the number of responses we are expecting. Now, we can't count those response
        /// messages w/o work.
        m_NumPendingCompletionResponse--;
    }

    if (m_TotalNumPixelsToProduce == m_CurrentPixelOffset)
    {
        if (!m_FailedJobs.empty())
        {
            auto failedPixelOffset2Count = m_FailedJobs.begin();
            SendNextFailedJob(pConnection, pRespMsg->GetThreadId(),
                              failedPixelOffset2Count->first, failedPixelOffset2Count->second);
            m_FailedJobs.erase(failedPixelOffset2Count);
        }
        else if (m_NumPendingCompletionResponse == 0)
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
            // Close the connection.
            // m_p_client_connection->Close();
            m_MyLisPtr = ListenerPtr(nullptr);

            /// Free the Apptag for this connection. All the reference count should get reset now and the command
            /// object should automatically get freed.
            pRespMsg->GetConnection()->FreeAppTag(pRespMsg->GetAppTag());
        }
    }
    else
    {
        /// Free the Apptag for this connection. All the reference count should get reset now and the command
        /// object should automatically get freed.
        pRespMsg->GetConnection()->FreeAppTag(pRespMsg->GetAppTag());

        if (!m_FailedJobs.empty())
        {
            auto failedPixelOffset2Count = m_FailedJobs.begin();
            SendNextFailedJob(pConnection, pRespMsg->GetThreadId(),
                              failedPixelOffset2Count->first, failedPixelOffset2Count->second);
            m_FailedJobs.erase(failedPixelOffset2Count);
        }
        else
        {
            /// Decay the workload a bit until we hit SchedulingPolicyParam::Get().m_DynamicSchedulePixelChunkMin
            m_workload = (m_workload * SchedulingPolicyParam::Get().m_DynamicSchedulePixelChunkDecay) / 10000;
            if (m_workload < SchedulingPolicyParam::Get().m_DynamicSchedulePixelChunkMin)
            {
                m_workload = SchedulingPolicyParam::Get().m_DynamicSchedulePixelChunkMin;
            }
            /// Send some more to the thread.
            SendNextJob(pConnection, pRespMsg->GetThreadId(), pRespMsg->GetThreadId() + 1);
        }
    }

}

void SceneSchedulerDynamic::SendNextJob(TCPIOConnectionPtr p_connection, uint32_t startThread, uint16_t endThread)
{
    if (m_CurrentPixelOffset >= m_TotalNumPixelsToProduce)
    {
        return;
    }
    uint32_t numThread = endThread - startThread;
    uint32_t numberOfHwExecutionThreadsForCurrentWorker = endThread - startThread;
    PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(m_SceneId, numberOfHwExecutionThreadsForCurrentWorker);
    for (int hwExecutionThreadId = startThread; hwExecutionThreadId < endThread; ++hwExecutionThreadId)
    {
        int appTag = p_connection->AllocateAppTag();
        pixelProduceRequestMsg->Request(hwExecutionThreadId - startThread)->SetupAppTag(appTag);
        pixelProduceRequestMsg->Request(hwExecutionThreadId - startThread)->SetThreadId(hwExecutionThreadId);
        p_connection->RegisterNotification(appTag, m_MyLisPtr);
    }

    for (int hwExecutionThreadId = startThread; hwExecutionThreadId < endThread; ++hwExecutionThreadId)
    {
        if (m_CurrentPixelOffset >= m_TotalNumPixelsToProduce)
        {
            break;
        }
        uint16_t endY  =  Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset).Y;
        uint16_t startX = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset).X;

        int workload = m_workload;
        if ((m_TotalNumPixelsToProduce - m_CurrentPixelOffset) <= m_workload)
        {
            workload = m_TotalNumPixelsToProduce - m_CurrentPixelOffset;
        }

        uint16_t startY = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset + workload - 1).Y;
        uint16_t endX = Pixel2XYMapper(m_NY, m_NX, m_CurrentPixelOffset + workload - 1).X;

        RELEASE_TRACE("Submitting Job to: " << (p_connection->GetUniqueHostName() + ":" + std::to_string(hwExecutionThreadId))
                      << ", Job Info: endY:" << endY << ", startY:" << startY << ", startX:" << startX
                      << ", endX:" << endX << "Num Pixels:" << workload);


        /// Update work set
        pixelProduceRequestMsg->Request(hwExecutionThreadId - startThread)->GenerateWork(startY, startX,  endY, endX);
        pixelProduceRequestMsg->Request(hwExecutionThreadId - startThread)->SetPixelDomain(m_CurrentPixelOffset, workload);


        /// Track the job
        ResourceTracker::Instance().TrackJob(p_connection->GetUniqueHostName(), hwExecutionThreadId, m_SceneId, m_CurrentPixelOffset, workload);

        m_NumPendingCompletionResponse++;
        m_CurrentPixelOffset += workload;
    }

    /// Send scene production message. Now we will wait for the response.
    p_connection->SendMsg(pixelProduceRequestMsg, ListenerPtr(nullptr));
}







