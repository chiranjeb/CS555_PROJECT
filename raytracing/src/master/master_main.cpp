#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "master/master_scheduler.hpp"

using namespace std;

int main(int argc, char *argv[])
{
   /// Read master properties
   std::string master_properties = argv[1];
   PropertiesReader  properties(master_properties);
   int master_port = std::stoi(properties["master_listening_port"]);
   int num_ray_scheduling_master_threads = std::stoi(properties["num_ray_scheduling_master_threads"]);
   int master_server_listening_q_d = std::stoi(properties["master_server_listening_q_depth"]);
   int scheduling_thread_q_depth = std::stoi(properties["scheduling_thread_q_depth"]);
   int scheduling_policy = std::stoi(properties["scheduling_policy"]);
   int static_schedule_policy = std::stoi(properties["static_scheduling_policy"]);

   int dynamic_schedule_initial_pixel_chunk_max = std::stoi(properties["dynamic_schedule_initial_pixel_chunk_max"]);
   int dynamic_schedule_initial_pixel_chunk_max_throttle_factor = std::stoi(properties["dynamic_schedule_initial_pixel_chunk_max_throttle_factor"]);
   int dynamic_schedule_pixel_chunk_decay = std::stoi(properties["dynamic_schedule_pixel_chunk_decay"]);
   int dynamic_schedule_pixel_chunk_min = std::stoi(properties["dynamic_schedule_pixel_chunk_min"]);


   RELEASE_TRACE("[Master Properties] Starting master on: " << master_port);
   RELEASE_TRACE("[Master Properties] Number of scheduling threads: " << num_ray_scheduling_master_threads);
   RELEASE_TRACE("[Master Properties] Master server listening queue depth: " << master_server_listening_q_d);
   RELEASE_TRACE("[Master Properties] Master scheduling threadQ depth: " << scheduling_thread_q_depth);
   RELEASE_TRACE("[Master Properties] Master scheduling policy(static-0, dynamic-1): " << scheduling_policy);
   RELEASE_TRACE("[Master Properties] Master static schedule policy: " << static_schedule_policy);

   RELEASE_TRACE("[Master Properties] dynamic schedule initial pixel chunk_maxx: " << dynamic_schedule_initial_pixel_chunk_max);
   RELEASE_TRACE("[Master Properties] dynamic schedule initial pixel chunk_max_throttle_factor: " << dynamic_schedule_initial_pixel_chunk_max_throttle_factor);
   RELEASE_TRACE("[Master Properties] dynamic schedule pixel chunk decay: " << dynamic_schedule_pixel_chunk_decay);
   RELEASE_TRACE("[Master Properties] dynamic schedule pixel chunk min: " << dynamic_schedule_pixel_chunk_min);


   /// Update scheduling policy.
   SchedulingPolicyParam::Get().SetSchedulingPolicy(scheduling_policy);
   SchedulingPolicyParam::Get().SetStaticSchedulingPolicy(static_schedule_policy);
   SchedulingPolicyParam::Get().SetDynamicScheduleInitialPixelChunkMax(dynamic_schedule_initial_pixel_chunk_max);
   SchedulingPolicyParam::Get().SetDynamicScheduleInitialPixelChunkMaxThrottleFactor(dynamic_schedule_initial_pixel_chunk_max_throttle_factor);
   SchedulingPolicyParam::Get().SetDynamicSchedulePixelChunkDecay(dynamic_schedule_pixel_chunk_decay);
   SchedulingPolicyParam::Get().SetDynamicSchedulePixelChunkMin(dynamic_schedule_pixel_chunk_min);

   /// Initialize the master
   Master::Instantiate(num_ray_scheduling_master_threads, scheduling_thread_q_depth);

   /// Create master server
   TransportMgr::Instance().CreateTCPServer(master_port, master_server_listening_q_d, Master::Instance().GetLis());

   /// Put the master to sleep
   while (1)
   {
      /// put the main thread to sleep.
      std::this_thread::sleep_for(std::chrono::milliseconds(60000));
   }
}


