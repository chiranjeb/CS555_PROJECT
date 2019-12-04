#include "scheduler_base.hpp"
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


void SchedulerBase::ProcessMsg(MsgPtr msg)
{
    switch (msg->GetId())
    {
        case MsgIdSceneProduceRequest:
            OnSceneProduceRequestMsg(msg);
            break;

        case MsgIdXmitStatus:
            OnXmitStatus(msg);
            break;

        case MsgIdTCPConnectionException:
            OnTCPConnectionException(msg);
            break;

        default:
            break;
    }
}

void SchedulerBase::OnXmitStatus(MsgPtr msg)
{
    TCPSendStatusMsgPtr sendStatusMsg = std::dynamic_pointer_cast<TCPSendStatusMsg>(msg);
    if (sendStatusMsg->GetErrorCode() == ERR_TRANSPORT_CONNECTION_CLOSED)
    {
        if (sendStatusMsg->GetWireMessageSent() == MsgIdSceneProduceRequest)
        {
            /// Handle connection transmit status... We don't need to worry about not able to send the scene
            /// file to this host. We should still be able to send the remaining pixels to the rest of the threads.
            DEBUG_TRACE_APPLICATION("SceneSchedulerDynamic::OnXmitStatus Start")
        }
    }
}


void SchedulerBase::OnTCPConnectionException(MsgPtr msg)
{
    DEBUG_TRACE_APPLICATION("SceneSchedulerDynamic::OnTCPConnectionException Start")
    ResourceTracker::Instance().Dump();
    TCPConnectionExceptionMsgPtr connectionException = std::dynamic_pointer_cast<TCPConnectionExceptionMsg>(msg);
    ResourceTracker::Instance().RemoveFailedJobs(connectionException->GetConnection()->GetUniqueHostName(), m_SceneId, m_FailedJobs);

    for (auto iter = m_FailedJobs.begin(); iter != m_FailedJobs.end(); iter++)
    {
        DEBUG_TRACE_APPLICATION("SceneSchedulerDynamic::OnTCPConnectionException" << "offset:" << iter->first << "count:" << iter->second);
    }
    DEBUG_TRACE_APPLICATION("SceneSchedulerDynamic::OnTCPConnectionException End")
}



void SchedulerBase::SendNextFailedJob(TCPIOConnectionPtr p_connection, uint32_t pipelineId, uint32_t failedPixelOffset, uint32_t workload)
{
    PixelProduceRequestMsgPtr pixelProduceRequestMsg = std::make_shared<PixelProduceRequestMsg>(m_SceneId, 1);
    uint16_t endY  =  Pixel2XYMapper(m_NY, m_NX, failedPixelOffset).Y;
    uint16_t startX = Pixel2XYMapper(m_NY, m_NX, failedPixelOffset).X;

    uint16_t startY = Pixel2XYMapper(m_NY, m_NX, failedPixelOffset + workload - 1).Y;
    uint16_t endX = Pixel2XYMapper(m_NY, m_NX, failedPixelOffset + workload - 1).X;


    RELEASE_TRACE("Submitting Job to: " << (p_connection->GetUniqueHostName() + ":" + std::to_string(pipelineId))
                  << ", Job Info: endY:" << endY << ", startY:" << startY << ", startX:" << startX
                  << ", endX:" << endX << "Num Pixels:" << workload);


    int appTag = p_connection->AllocateAppTag();

    /// Update work set
    pixelProduceRequestMsg->Request(0)->GenerateWork(startY, startX,  endY, endX);
    pixelProduceRequestMsg->Request(0)->SetPixelDomain(failedPixelOffset, workload);
    pixelProduceRequestMsg->Request(0)->SetupAppTag(appTag);
    pixelProduceRequestMsg->Request(0)->SetPipelineId(pipelineId);


    p_connection->RegisterNotification(appTag, m_MyLisPtr);
    m_NumPendingCompletionResponse++;

    /// Track the job
    ResourceTracker::Instance().TrackJob(p_connection->GetUniqueHostName(), pipelineId, m_SceneId, failedPixelOffset, workload);

    /// Send scene production message. Now we will wait for the response.
    p_connection->SendMsg(pixelProduceRequestMsg, ListenerPtr(nullptr));
}


void SchedulerBase::OnSceneProduceRequestMsg(MsgPtr msg)
{
    SceneProduceRequestMsgPtr pRequestMsg = std::dynamic_pointer_cast<SceneProduceRequestMsg>(msg);
    std::vector<ResourceEntryPtr> & workerList = ResourceTracker::Instance().GetHostWorkers();
    ErrorCode_t status = ERR_CLUSTER_INIT_IN_POGRESS;
    int appTag = pRequestMsg->GetAppTag();
    if (workerList.size() != 0)
    {
        m_NumPendingCompletionResponse = 0;
        m_NX = pRequestMsg->GetNX();
        m_NY = pRequestMsg->GetNY();
        m_RPP = pRequestMsg->GetRPP();
        m_SceneId = pRequestMsg->GetSceneId();

        DEBUG_TRACE("sceneDescriptorPtr->GetNY(): " << m_NX << ", sceneDescriptorPtr->GetNX():" << m_NY << ", m_workerList.size()" << workerList.size());
        m_p_client_connection = pRequestMsg->GetConnection();

        pRequestMsg->SetAppTag(0);

        ///We need to reserialize the first few bytes....
        pRequestMsg->Repack();

        /// Let's distribute the scene file. This also helps us not doing any serialization/deserialization of the message.
        for (int index = 0; index < workerList.size(); ++index)
        {
            TransportMgr::Instance().FindConnection(workerList[index]->m_UniqueHostName)->SendMsg(pRequestMsg, ListenerPtr(m_MyLisPtr));
        }

        /// Let's first generate sequential pixel workload
        KickOffSceneScheduling();

        status = STATUS_SUCCESS;
    }

    /// all the tasks are scheduled. Send the acknowledgement to the client that the request has been accepted.
    SceneProduceRequestAckMsgPtr sceneProduceRequestAckMsgPtr = std::make_shared<SceneProduceRequestAckMsg>(appTag, status);

    /// Send the response back to the client.
    pRequestMsg->GetConnection()->SendMsg(sceneProduceRequestAckMsgPtr, ListenerPtr(nullptr));
}


