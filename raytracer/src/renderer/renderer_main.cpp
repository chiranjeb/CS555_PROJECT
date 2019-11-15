#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "renderer/renderer_scheduler.hpp"

using namespace std;
static const int SERVER_LISTENING_Q_DEPTH = 40;

int main(int argc, char *argv[])
{
   std::string renderer_properties = argv[1];
   PropertiesReader  properties(renderer_properties);
   int renderer_port = std::stoi(properties["renderer_listening_port"]);

   // Start the Renderer Scheduler.
   RendererScheduler::Instance().Start();

   // Start a server
   TransportMgr::Instance().CreateTCPServer(renderer_port, SERVER_LISTENING_Q_DEPTH, *RendererScheduler::Instance().GetThrdListener());


   while (1)
   {
      // Submit jobs
      //RendererCli::Instance().Start();
   }
}


