#pragma once
#include "wire_msg.hpp"

/**
 * This wire message is sent by the worker node to scheduler to 
 * register itself as a worker. 
 *  
 */
class WorkerRegistrationMsg : public WireMsg
{
public:
   /////////////////////////////////////////////////////////////////////////
   /// WorkerRegistrationMsg message constructor
   /////////////////////////////////////////////////////////////////////////
   WorkerRegistrationMsg(std::string hostname, int port, uint16_t hw_thread_concurrency_level) :
      WireMsg(MsgIdWorkerRegistrationRequest), m_hostname(hostname), m_Port(port), m_hw_thread_concurrency_level(hw_thread_concurrency_level)
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationMsg: Constructor");
   }
   WorkerRegistrationMsg() : WireMsg(MsgIdWorkerRegistrationRequest), m_hostname(""), m_Port(0), m_hw_thread_concurrency_level(0)
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationMsg: Constructor");
   }

   uint16_t GetNumHwExecutionThread()
   {
      return m_hw_thread_concurrency_level;
   }

   /// Custom Message serializer
   void Pack(std::ostream& ostrm);

   ///  Custom message deserializer
   void Unpack(std::istream& istrm);

   ~WorkerRegistrationMsg()
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationMsg: Destructor");
   }

   std::string m_hostname;   //  server name
   int m_Port;                // server port
   uint16_t m_hw_thread_concurrency_level;
};

///@@@ Worker registration response message
class WorkerRegistrationRespMsg : public WireMsg
{
public:
   /////////////////////////////////////////////////////////////////////////
   /// WorkerRegistrationRespMsg message constructor
   /////////////////////////////////////////////////////////////////////////
   WorkerRegistrationRespMsg(ErrorCode_t errorCode) :
      WireMsg(MsgIdWorkerRegistrationResponse), m_ErrorCode(errorCode)
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationRespMsg: Constructor");
   }

   WorkerRegistrationRespMsg() :
      WireMsg(MsgIdWorkerRegistrationResponse)
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationRespMsg: Constructor");
   }

   /// Custom Message serializer
   void Pack(std::ostream& ostrm);

   ///  Custom message deserializer
   void Unpack(std::istream& istrm);


   ~WorkerRegistrationRespMsg()
   {
      DEBUG_TRACE_WIRE_MSG("WorkerRegistrationRespMsg: Destructor");
   }

   int m_ErrorCode;  // Error code
};




typedef std::shared_ptr<WorkerRegistrationMsg> WorkerRegistrationMsgPtr;
typedef std::shared_ptr<WorkerRegistrationRespMsg> WorkerRegistrationRespMsgPtr;
