#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include<map>

class TCPIOConnection;
class SceneSchedulerDynamic : public Command
{
public:
    /// Constructor
    SceneSchedulerDynamic(BlockingMsgQPtr pQ) : Command(pQ)
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

    /// Scene produce request message handler.
    void OnSceneProduceRequestMsg(MsgPtr msg);

    /// Pixel produce response message.
    void OnPixelProduceResponseMsg(MsgPtr msg);

    /// Different chunking of the work.
    void KickOffSceneScheduling();

    /// Send next job
    void SendNextJob(TCPIOConnection *p_connection, uint32_t startThread, uint16_t endThread);

    /// attributes
    uint32_t m_NumPendingCompletionResponse;
    TCPIOConnection *m_p_client_connection;
    std::size_t m_SceneId;
    uint32_t m_NX, m_NY;
    uint32_t m_CurrentPixelOffset;
    uint32_t m_TotalNumPixelsToProduce;
    uint32_t m_workload;
};

typedef std::shared_ptr<SceneSchedulerDynamic> SceneSchedulerDynamicPtr;
