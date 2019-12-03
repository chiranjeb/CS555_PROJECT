#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include<map>

class SchedulerBase : public Command
{
public:
    /// Constructor
    SchedulerBase(BlockingMsgQPtr pQ) : Command(pQ)
    {
    }

    /// Process message
    virtual void ProcessMsg(MsgPtr msg);

    /// Handle xmit status.
    void OnXmitStatus(MsgPtr msg);

    /// TCP Connection exception message handler.
    void OnTCPConnectionException(MsgPtr msg);

    /// Send next failed job
    void SendNextFailedJob(TCPIOConnectionPtr p_connection, uint32_t threadId, uint32_t failedPixelOffset, uint32_t workload);

    /// 
    void OnSceneProduceRequestMsg(MsgPtr msg);

    /// Different chunking of the work.
    virtual void KickOffSceneScheduling(){}

protected:
    /// attributes
    uint32_t m_TotalNumPixelsToProduce;
    uint32_t m_NumPendingCompletionResponse;
    TCPIOConnectionPtr m_p_client_connection;
    std::size_t m_SceneId;
    uint32_t m_NX, m_NY, m_RPP;
    std::map<uint32_t, uint32_t> m_FailedJobs;
};
