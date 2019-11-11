#pragma once
#include "wire_msg.hpp"

/**
 * This wire message is sent by the peer node to discovery node 
 * to register itself with the overlay.
 *  
 */
class WorkerRegistrationMsg : public WireMsg
{
   public:
   /** 
   * WorkerRegistrationMsg message constructor
   *  
   */
   WorkerRegistrationMsg(): WireMsg(MsgIdWorkerRegistrationRequest)
   {
      m_Test1 = 1;
      m_Test2 = 0;
   }

   WorkerRegistrationMsg() : WireMsg(MsgIdWorkerRegistrationRequest)
   {
   }

   /**
   *  Custom Message serializer
   *  @throws IOException if an I/O error occurs.
   *  @param out Output stream where the message is being
   *             serialized to
   */
   void Pack(std::ostream out)
   {
      WireMsg::Pack(out);
      out << m_Test1;
      out << m_Test2;
      //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, "WorkerRegistrationMsg:Pack -  GUID: " + m_GUID.toString()+   ", m_TCPServerId: " + m_TCPServerId.toString());
   }

   /**
   *  Custom message deserializer
   *  @throws IOException if an I/O error occurs.
   *  @param in Input stream from which the message is being
   *            deserialized.
   */
   void Unpack(std::istream in)throws IOException
   {
      super.Unpack(in);
      m_GUID = new GUID("");
      m_GUID.Unpack(in);
      m_TCPServerId = new UniqueTCPServerId();
      m_TCPServerId.Unpack(in);

      TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, "WorkerRegistrationMsg:UnPack -  GUID:" + m_GUID.toString()+   "m_TCPServerId:" + m_TCPServerId.toString());
   }


   int m_Test1;   // GUID / server identifier
   int m_Test2;   // TCP server info. IP and port.
}

