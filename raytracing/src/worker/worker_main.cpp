#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "worker.hpp"

using namespace std;

int main(int argc, char *argv[])
{
   // Setup master info
   std::string master_properties = argv[1];
   PropertiesReader  properties(master_properties);
   std::string master_address = properties["master_host"];
   int master_port = std::stoi(properties["master_listening_port"]);
   Worker::Instance().SetupMasterInfo(master_address, master_port);

   // Start the worker.
   Worker::Instance().Start();

   // Start a server
   const int SERVER_LISTENING_Q_DEPTH = 40;
   TransportMgr::Instance().CreateTCPServer(0, SERVER_LISTENING_Q_DEPTH, *Worker::Instance().GetThrdListener());

   while (1)
   {
      //put the main thread to sleep.
      std::this_thread::sleep_for(std::chrono::milliseconds(60000));
   }
}

