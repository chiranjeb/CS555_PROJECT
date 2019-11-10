#include <iostream>
#include "renderer_scheduler.hpp"

void RendererScheduler::Run()
{
   std::cerr << "Started Renderer Scheduler thread" << std::endl;
   while (1)
   {
       std::cerr << "Running Scheduler thread" << std::endl;
       std::this_thread::sleep_for(std::chrono::milliseconds(10));
   }
}


