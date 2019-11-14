#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOConnection;
class Worker : public MsgQThread
{
public:
   Worker() : MsgQThread("Worker", 128)
   {
   }
   /// Get the Worker
   static Worker& Instance()
   {
      static Worker s_Worker;
      return s_Worker;
   }


   /// Start the worker thread
   void Start()
   {
      m_thread = new std::thread(&Worker::Run, *this);
   }

protected:

   /// Run the worker thread
   void Run();

   /// Connection establishment response msg
   void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

   TCPIOConnection *m_p_TCPIOConnection;
};
