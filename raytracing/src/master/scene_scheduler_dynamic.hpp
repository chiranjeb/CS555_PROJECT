#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include "scheduler_base.hpp"
#include<map>

class SceneSchedulerDynamic : public SchedulerBase
{
public:
    /// Constructor
    SceneSchedulerDynamic(BlockingMsgQPtr pQ) : SchedulerBase(pQ)
    {
    }

    /// Actual Scheduler thread
    void ProcessMsg(MsgPtr msg);

    /// Destructor
    ~SceneSchedulerDynamic()
    {
        DEBUG_TRACE("***SceneSchedulerDynamic - Destructor*****" << std::hex << this)
    }

protected:
    /// Pixel produce response message.
    void OnPixelProduceResponseMsg(MsgPtr msg);

    /// Handle xmit status.
    void OnXmitStatus(MsgPtr msg);

    /// Different chunking of the work.
    void KickOffSceneScheduling();

    /// Send next job
    void SendNextJob(TCPIOConnectionPtr p_connection, uint32_t startThread, uint16_t endThread);

    /// attributes
    uint32_t m_CurrentPixelOffset;
    uint32_t m_workload;
};

typedef std::shared_ptr<SceneSchedulerDynamic> SceneSchedulerDynamicPtr;
