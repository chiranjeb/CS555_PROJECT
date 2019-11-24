#pragma once
#include<iostream>
#include<string>
#include<memory>
#include<mutex>
#include <vector>

struct Pixel2XYMapper
{
   Pixel2XYMapper(int Ny, int Nx, int pixelPos)
   {
      Y = Ny - ((pixelPos/ Nx) + 1) ;
      X = pixelPos % Nx;
   }
   uint16_t X;
   uint16_t Y;
};


struct ResourceEntry
{
   ResourceEntry(std::string host_name, uint16_t available_hw_execution_thread) :
      m_unique_host_name(host_name), m_available_hw_execution_thread(available_hw_execution_thread)
   {
   }

   std::string m_unique_host_name;
   int m_available_hw_execution_thread;
};

typedef std::shared_ptr<ResourceEntry> ResourceEntryPtr;

class ResourceTracker
{
public:
   ResourceTracker()
   {
      m_total_num_hw_threads = 0;
   }

   static ResourceTracker& Instance();

   uint16_t GetTotalNumberOfHwThreads()
   {
      return m_total_num_hw_threads;
   }

   std::vector<ResourceEntryPtr>& GetWorkers()
   {
      return m_workerlist;
   }

   void AddWorker(std::string host_name, uint16_t available_hw_execution_thread);

protected:
   uint32_t m_total_num_hw_threads;
   std::mutex m_Mutex;
   std::vector<ResourceEntryPtr> m_workerlist;
};
