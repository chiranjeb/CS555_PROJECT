#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "worker.hpp"

using namespace std;

int main(int argc, char *argv[])
{
   /// Read properties. We need to read some master properties as well as our properties.
   std::string master_properties_file_name = argv[1];
   PropertiesReader  master_properties(master_properties_file_name);
   std::string master_address = master_properties["master_host"];
   int master_port = std::stoi(master_properties["master_listening_port"]);
   std::string known_scene_name = master_properties["known_scene_name"];
   int known_scene_nx = std::stoi(master_properties["known_scene_nx"]);
   int known_scene_ny = std::stoi(master_properties["known_scene_ny"]);
   int known_scene_ns = std::stoi(master_properties["known_scene_ns"]);
   std::string worker_properties_file_name = argv[2];

   PropertiesReader  worker_properties(worker_properties_file_name);
   int max_advertised_hw_concurrency_level = std::stoi(worker_properties["max_advertised_hw_concurrency_level"]);
   int worker_cmd_processor_q_depth = std::stoi(worker_properties["worker_cmd_processor_q_depth"]);
   int worker_server_listening_q_depth = std::stoi(worker_properties["worker_server_listening_q_depth"]);
   int scene_producer_q_depth = std::stoi(worker_properties["scene_producer_q_depth"]);


   RELEASE_TRACE("[Worker Properties] Bound to master: " << master_address);
   RELEASE_TRACE("[Worker Properties] Bound to master port master threads: " << master_port);
   RELEASE_TRACE("[Worker Properties] Worker server listening queue depth: " << worker_server_listening_q_depth);
   RELEASE_TRACE("[Worker Properties] Worker max advertised concurrency level(%): " << max_advertised_hw_concurrency_level);
   RELEASE_TRACE("[Worker Properties] Worker command processor queue depth: " << worker_cmd_processor_q_depth);
   RELEASE_TRACE("[Worker Properties] Scene producer Q depth: " << scene_producer_q_depth);
   RELEASE_TRACE("[Worker Properties] Scene producer Q depth: " << scene_producer_q_depth);

   

   /// Initialize and start the worker
   Worker::Initialize(master_address, 
                      master_port, 
                      worker_cmd_processor_q_depth,
                      max_advertised_hw_concurrency_level,
                      scene_producer_q_depth, known_scene_name, 
                      known_scene_nx, 
                      known_scene_ny, 
                      known_scene_ns);

   /// Start worker server
   TransportMgr::Instance().CreateTCPServer(0, worker_server_listening_q_depth, Worker::Instance().GetThrdListener());

   /// Put the main thread to sleep.
   while (1)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(60000));
   }
}

