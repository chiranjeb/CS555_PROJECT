#include "resource_tracker.hpp"
#include "framework/framework_includes.hpp"

ResourceTracker& ResourceTracker::Instance()
{
   static ResourceTracker s_ResourceTracker;
   return s_ResourceTracker;
}


void ResourceTracker::AddWorker(std::string host_name, uint16_t available_hw_execution_thread)
{
   std::unique_lock<std::mutex> lck(m_Mutex);
   m_workerlist.push_back(std::make_shared<ResourceEntry>(host_name, available_hw_execution_thread));

   m_total_num_hw_threads += available_hw_execution_thread;

   /// Dump the worker lists
   DEBUG_TRACE("worker list: " << m_workerlist.size());
   for (std::vector<ResourceEntryPtr>::iterator iter = m_workerlist.begin(); iter != m_workerlist.end(); iter++)
   {
      DEBUG_TRACE("worker: " << (*iter)->m_unique_host_name);
   }
}
