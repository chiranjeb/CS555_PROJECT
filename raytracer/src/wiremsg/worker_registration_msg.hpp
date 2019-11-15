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
   WorkerRegistrationMsg(std::string hostname, int port): 
      WireMsg(MsgIdWorkerRegistrationRequest), m_hostname(hostname), m_Port(port)
   {
       DEBUG_TRACE("WorkerRegistrationMsg: Constructor");
   }
   WorkerRegistrationMsg(): WireMsg(MsgIdWorkerRegistrationRequest), m_hostname(""), m_Port(0)
   {
       DEBUG_TRACE("WorkerRegistrationMsg: Constructor");
   }

   /////////////////////////////////////////////////////////////////////////
   /// Custom Message serializer
   /// 
   /// @param [ostrm] Output stream where the message is being serialized.
   /// 
   /////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream &ostrm)
   {
      WireMsg::Pack(ostrm);
      ostrm << m_hostname << " ";
      ostrm << m_Port << " ";
   }

   /////////////////////////////////////////////////////////////////////////
   ///  Custom message deserializer
   /// 
   ///  @param [istrm] Input stream from which the message is being deserialized.
   /// 
   /////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream &istrm)
   {
      WireMsg::Unpack(istrm);
      istrm >> m_hostname >> m_Port;
   }

   /// Dump routine
   void Dump()
   {
      DEBUG_TRACE("WorkerRegistrationMsg: " << "IPAddress: " << m_hostname << "Port: " << m_Port);
   }

   ~WorkerRegistrationMsg()
   {
       DEBUG_TRACE("WorkerRegistrationMsg: Destructor");
   }

   
   std::string m_hostname;   // GUID / server identifier
   int m_Port;                // TCP server info. IP and port.
};

///@@@ This message is for testing communication between 
class WorkerRegistrationRespMsg : public WireMsg
{
   public:
   /////////////////////////////////////////////////////////////////////////  
   /// WorkerRegistrationRespMsg message constructor
   ///////////////////////////////////////////////////////////////////////// 
   WorkerRegistrationRespMsg(ErrorCode_t errorCode): 
      WireMsg(MsgIdWorkerRegistrationResponse), m_ErrorCode(errorCode)
   {
       DEBUG_TRACE("WorkerRegistrationRespMsg: Constructor");
   }

   WorkerRegistrationRespMsg(): 
       WireMsg(MsgIdWorkerRegistrationResponse)
   {
       DEBUG_TRACE("WorkerRegistrationRespMsg: Constructor");
   }

   /////////////////////////////////////////////////////////////////////////
   /// Custom Message serializer
   /// 
   /// @param [ostrm] Output stream where the message is being serialized.
   /// 
   /////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream &ostrm)
   {
      WireMsg::Pack(ostrm);
      ostrm << m_ErrorCode << " ";
   }

   /////////////////////////////////////////////////////////////////////////
   ///  Custom message deserializer
   /// 
   ///  @param [istrm] Input stream from which the message is being deserialized.
   /// 
   /////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream &istrm)
   {
      WireMsg::Unpack(istrm);
      istrm >> m_ErrorCode;
   }

   /// Dump routine
   void Dump()
   {
      DEBUG_TRACE("WorkerRegistrationRespMsg: " << "m_ErrorCode: " << m_ErrorCode);
   }

   ~WorkerRegistrationRespMsg()
   {
       DEBUG_TRACE("WorkerRegistrationRespMsg: Destructor");
   }
   
   int m_ErrorCode;  // Error code
};




typedef std::shared_ptr<WorkerRegistrationMsg> WorkerRegistrationMsgPtr;
typedef std::shared_ptr<WorkerRegistrationRespMsg> WorkerRegistrationRespMsgPtr;
