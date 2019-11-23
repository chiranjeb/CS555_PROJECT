#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include<vector>


class MasterScheduler : public MsgQThread
{
public:
   static const int SCHEDULER_MSG_Q_DEPTH = 128;
   MasterScheduler() : MsgQThread("MasterScheduler", SCHEDULER_MSG_Q_DEPTH)
   {
   }

   // Start the scheduler thread
   void Start();

private:
   // Actual Scheduler thread
   void Run();

   // On unsolicited TCP receive message.
   void OnTCPRecvMsg(MsgPtr msg);

   void OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr);

   void OnSceneProduceRequestMsg(WireMsgPtr wireMsgPtr);
};

class Master
{
   static const int NUM_RAY_SCHEDULER = 32;
public:
   Master()
   {
      m_ThreadPoolLis.Construct(m_MasterScheduler, NUM_RAY_SCHEDULER);
   }

   // Get the Master scheduler
   static Master& Instance()
   {
      static Master s_Master;
      return s_Master;
   }

   void Start()
   {
      for (int index = 0; index < NUM_RAY_SCHEDULER; ++index)
      {
         // Start the Master Scheduler.
         m_MasterScheduler[index].Start();
      }
   }

   Listener* GetLis()
   {
      return &m_ThreadPoolLis;
   }

   void AddWorker(std::string &worker);

   std::vector<std::string>& GetWorkerList()
   {
      return m_workerlist;
   }

protected:
   MasterScheduler m_MasterScheduler[NUM_RAY_SCHEDULER];
   MsgQThreadPoolLis<MasterScheduler> m_ThreadPoolLis;

   std::vector<std::string> m_workerlist;
   std::mutex m_Mutex;
};
