#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include "scheduler_base.hpp"
#include<map>

class TCPIOConnection;
class SceneSchedulerStatic : public SchedulerBase
{
public:
    /// Constructor
    SceneSchedulerStatic(BlockingMsgQPtr pQ) : SchedulerBase(pQ)
    {
    }

    /// Actual Scheduler thread
    void ProcessMsg(MsgPtr msg);

    /// Destructor
    ~SceneSchedulerStatic()
    {
        DEBUG_TRACE("***SceneSchedulerStatic - Destructor*****" << std::hex << this)
    }

protected:

    /// Scene produce request message handler.
    void OnSceneProduceRequestMsg(MsgPtr msg);

    /// Pixel produce response message.
    void OnPixelProduceResponseMsg(MsgPtr msg);

    /// Different chunking of the work.
    void KickOffSceneScheduling();

    /// Handle xmit status.
    void OnXmitStatus(MsgPtr msg);

    /// TCP Connection exception message handler.
    void OnTCPConnectionException(MsgPtr msg);

};

typedef std::shared_ptr<SceneSchedulerStatic> SceneSchedulerStaticPtr;
