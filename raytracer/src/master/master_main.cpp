#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "master/master_scheduler.hpp"

using namespace std;
static const int SERVER_LISTENING_Q_DEPTH = 40;

int main(int argc, char *argv[])
{
   std::string master_properties = argv[1];
   PropertiesReader  properties(master_properties);
   int master_port = std::stoi(properties["master_listening_port"]);

   // Start the Master Scheduler.
   MasterScheduler::Instance().Start();

   // Start a server
   TransportMgr::Instance().CreateTCPServer(master_port, SERVER_LISTENING_Q_DEPTH, *MasterScheduler::Instance().GetThrdListener());


   while (1)
   {
      // Submit jobs
      //MasterCli::Instance().Start();
   }
}


