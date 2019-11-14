#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "worker.hpp"

using namespace std;

int main(int argc, char *argv[])
{
   std::string renderer_properties = argv[1];
   std::cerr << "renderer_properties: " << renderer_properties << endl;
   PropertiesReader  properties(renderer_properties);
   std::string renderer_address = properties["renderer_host"];
   int renderer_port = std::stoi(properties["renderer_listening_port"]);


   // Start the Renderer Scheduler.
   Worker::Instance().Start();

   // Establish connection with the renderer.
   TransportMgr::Instance().EstablishNewConnection(renderer_address,  renderer_port,  Worker::Instance().GetThrdListener(), true);


   while (1)
   {
      //put the main thread to sleep.
   }
}

