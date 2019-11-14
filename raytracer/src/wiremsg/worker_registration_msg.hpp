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
   WorkerRegistrationMsg(std::string ipAddress, int port): 
      WireMsg(MsgIdWorkerRegistrationRequest), m_IPAddress(ipAddress), m_Port(port)
   {
   }

   WorkerRegistrationMsg(): WireMsg(MsgIdWorkerRegistrationRequest), m_IPAddress(""), m_Port(0)
   {
   }

   /**
   *  Custom Message serializer
   *  @throws IOException if an I/O error occurs.
   *  @param out Output stream where the message is being
   *             serialized to
   */
   void Pack(std::ostream &ostrm)
   {
      std::cerr << "WorkerRegistrationMsg:Pack" << std::endl;
      WireMsg::Pack(ostrm);
      ostrm << m_IPAddress << " ";
      ostrm << m_Port << " ";
   }

   /**
   *  Custom message deserializer
   *  @throws IOException if an I/O error occurs.
   *  @param in Input stream from which the message is being
   *            deserialized.
   */
   void Unpack(std::istream &istrm)
   {
      WireMsg::Unpack(istrm);
      istrm >> m_IPAddress >> m_Port;

      //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, "WorkerRegistrationMsg:UnPack -  GUID:" + m_GUID.toString()+   "m_TCPServerId:" + m_TCPServerId.toString());
   }

   void Dump()
   {
      std::cerr << "WorkerRegistrationMsg: " << "IPAddress:" << m_IPAddress << "Port" << m_Port << std::endl;
   }


   std::string m_IPAddress;   // GUID / server identifier
   int m_Port;   // TCP server info. IP and port.
};



typedef std::shared_ptr<WorkerRegistrationMsg> WorkerRegistrationMsgPtr;
