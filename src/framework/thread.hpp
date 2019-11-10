#pragma once
#include <iostream>
#include <thread>


class Thread
{
public:
   Thread()
   {
   }

   virtual void Run()
   {
      while (1)
      {
         std::cerr << "Not Implemented";
      }
   }

   virtual void Start()
   {
      m_thread = new std::thread(&Thread::ThreadRunner, *this);
   }

protected:
   void ThreadRunner()
   {
      Run();
   }
   std::thread *m_thread;
};

