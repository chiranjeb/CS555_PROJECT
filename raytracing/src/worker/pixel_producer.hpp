#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/tcp_io_connection.hpp"

class PixelProducer : public Command
{
public:
   PixelProducer(BlockingMsgQPtr pQ, TCPIOConnectionPtr p_clientConnection, TCPIOConnectionPtr p_MasterConnection, uint16_t requestIndex) :
      Command(pQ), m_p_clientConnection(p_clientConnection), m_pConnectionToMaster(p_MasterConnection), m_requestIndex(requestIndex)
   {
   }

   /// Set the connection
   void SetConnection(TCPIOConnectionPtr p_connection)
   {
      m_p_clientConnection = p_connection;
   }

protected:

   /// Run the worker thread
   void ProcessMsg(MsgPtr msg);

   void OnPixelProduceRequestMsg(MsgPtr msg);

   TCPIOConnectionPtr m_p_clientConnection;
   TCPIOConnectionPtr m_pConnectionToMaster;

   uint16_t m_requestIndex;
};

typedef std::shared_ptr<PixelProducer> PixelProducerPtr;
