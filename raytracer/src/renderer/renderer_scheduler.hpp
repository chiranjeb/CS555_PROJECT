#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class RendererScheduler : public MsgQThread
{
public:
   RendererScheduler() : MsgQThread("RendererScheduler", 128)
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

};
