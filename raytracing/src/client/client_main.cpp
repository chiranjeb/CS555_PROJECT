#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "client.hpp"

using namespace std;

int main(int argc, char *argv[])
{
   // Setup master info
   std::string master_properties = argv[1];
   PropertiesReader  properties(master_properties);
   std::string master_address = properties["master_host"];
   int master_port = std::stoi(properties["master_listening_port"]);
   Client::Instance().SetupMasterInfo(master_address, master_port);


   std::string scene_name = argv[2];
   Client::Instance().SetupSceneName(scene_name);

   // Start the worker.
   Client::Instance().Start();

   // Start a server
   const int SERVER_LISTENING_Q_DEPTH = 10;
   TransportMgr::Instance().CreateTCPServer(0, SERVER_LISTENING_Q_DEPTH, *Client::Instance().GetThrdListener());

   while (1)
   {
      //put the main thread to sleep.
      std::this_thread::sleep_for(std::chrono::milliseconds(60000));
   }
}

