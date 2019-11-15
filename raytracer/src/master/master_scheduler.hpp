#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include<list>


class MasterScheduler : public MsgQThread
{
    static const int SCHEDULER_MSG_Q_DEPTH = 128;
public:
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
   void Start()
   {
      m_thread = new std::thread(&MasterScheduler::Run, *this);
   }

private:
   // Actual Scheduler thread
   void Run();

   // On unsolicited TCP receive message.
   void OnTCPRecvMsg(MsgPtr msg);

   void OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr);


   std::list<std::string> m_workerlist;
};
