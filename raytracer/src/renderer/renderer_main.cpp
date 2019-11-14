#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "renderer/renderer_scheduler.hpp"

using namespace std;
static const int SERVER_LISTENING_Q_DEPTH = 40;

int main()
{
   // Start the Renderer Scheduler.
   RendererScheduler::Instance().Start();

   // Start a server
   TransportMgr::Instance().CreateTCPServer(8050, 40, *RendererScheduler::Instance().GetThrdListener());


   while (1)
   {
      // Submit jobs
      //RendererCli::Instance().Start();
   }
}


