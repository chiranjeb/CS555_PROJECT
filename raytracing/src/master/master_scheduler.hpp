#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "resource_tracker.hpp"
#include <vector>

///////////////////////////////////////////////////////////////////////////////////
///
///  Scheduling Policy Parameter: This is the master scheduler policy parameter.
///  
////////////////////////////////////////////////////////////////////////////////////
struct SchedulingPolicyParam
{
   /// Return scheduling policy parameter.
   static SchedulingPolicyParam& Get();

   bool ConfiguredAsStaticScheduler()
   {
      return (m_Policy == 0);
   }

   int m_Policy; /// Scheduling policy. 0 - for static scheduler. 1 - for dynamic schedule
};

///////////////////////////////////////////////////////////////////////////////////
///
///  MasterSchedulerThread: This is the master scheduler thread. There are 'N'
///  configurable master scheduler threads in the system. Each new scene producing 
///  request will be handled by a seperate scheduling thread. This is to make sure 
///  better Qos for the incoming scene producing request. 
///                   
////////////////////////////////////////////////////////////////////////////////////
class MasterSchedulerThread : public MsgQThread
{
public:
   /// Constructor
   MasterSchedulerThread(int scheduling_thread_q_depth);

   /// Start the master scheduler thread
   void Start();

private:
   /// Actual Scheduler thread
   void Run();

   /// On unsolicited TCP receive message.
   void OnTCPRecvMsg(MsgPtr msg);

   /// worker registration request
   void OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr);

   /// Scene produce request message
   void OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr);
};

class Master
{
public:
   /// Instantantiate master
   static void Instantiate(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth, int scheduling_policy);

   // Get the Master scheduler
   static Master& Instance();

   /// Start master
   void Start();

   /// Return listener for the unsolicited message notification.
   Listener* GetLis()
   {
      return &m_ThreadPoolLis;
   }

protected:
   /// Constructor
   Master(int num_ray_scheduling_master_threads, int scheduling_thread_q_depth);

   MasterSchedulerThread **m_pMasterSchedulerThread;
   int m_num_ray_scheduling_master_threads;
   MsgQThreadPoolLis<MasterSchedulerThread> m_ThreadPoolLis;
};
