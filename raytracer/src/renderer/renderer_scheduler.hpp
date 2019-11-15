#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"


class RendererScheduler : public MsgQThread
{
    static const int SCHEDULER_MSG_Q_DEPTH = 128;
public:
   RendererScheduler() : MsgQThread("RendererScheduler", SCHEDULER_MSG_Q_DEPTH)
   {
   }
   // Get the Renderer scheduler
   static RendererScheduler& Instance()
   {
      static RendererScheduler s_Scheduler;
      return s_Scheduler;
   }

   // Start the scheduler thread
   void Start()
   {
      m_thread = new std::thread(&RendererScheduler::Run, *this);
   }

   // Actual Scheduler thread
   void Run();

   // On unsolicited TCP receive message.
   void OnTCPRecvMsg(MsgPtr msg);

};
