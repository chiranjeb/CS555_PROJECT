#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include<vector>


class MasterScheduler : public MsgQThread
{
public:
   static const int SCHEDULER_MSG_Q_DEPTH = 128;
   MasterScheduler() : MsgQThread("MasterScheduler", SCHEDULER_MSG_Q_DEPTH)
   {
   }
   // Get the Master scheduler
   static MasterScheduler& Instance()
   {
      static MasterScheduler s_Scheduler;
      return s_Scheduler;
   }

   // Start the scheduler thread
   void Start();

private:
   // Actual Scheduler thread
   void Run();

   // On unsolicited TCP receive message.
   void OnTCPRecvMsg(MsgPtr msg);

   void OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr);

   void OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr);

   std::vector<std::string> m_workerlist;
};
