#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include<map>

class TCPIOConnection;
class SceneScheduler : public Command
{
public:
   SceneScheduler(BlockingMsgQPtr pQ) : Command(pQ)
   {
   }

   /// Actual Scheduler thread
   void ProcessMsg(MsgPtr msg);
protected:

   /// Scene produce request message handler.
   void OnSceneProduceRequestMsg(MsgPtr msg);

   /// Pixel produce response message.
   void OnPixelProduceResponseMsg(MsgPtr msg);

   /// Different chunking of the work.
   void KickOffSceneScheduling();

   uint32_t m_NumPendingCompletionResponse;
   std::map<std::string, bool> m_workOrder;
   TCPIOConnection *m_p_client_connection;
   std::vector<std::string> m_workerList;

   std::size_t m_SceneId;
   uint32_t  m_NX, m_NY;
   uint32_t m_TotalNumPixelsToProduce;
   uint32_t m_CurrentPixelOffset;
};

typedef std::shared_ptr<SceneScheduler> SceneSchedulerPtr;
