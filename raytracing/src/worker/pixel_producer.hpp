#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOConnection;
class PixelProducer : public Command
{
public:
   PixelProducer(BlockingMsgQPtr pQ, TCPIOConnection *p_clientConnection, uint16_t requestIndex) :
      Command(pQ), m_p_clientConnection(p_clientConnection), m_requestIndex(requestIndex)
   {
   }

   /// Set the connection
   void SetConnection(TCPIOConnection *p_connection)
   {
      m_p_clientConnection = p_connection;
   }

protected:

   /// Run the worker thread
   void ProcessMsg(MsgPtr msg);

   void OnPixelProduceRequestMsg(MsgPtr msg);

   void OnSceneProduceDone(MsgPtr msg);

   TCPIOConnection *m_p_clientConnection;

   uint16_t m_requestIndex;
};

typedef std::shared_ptr<PixelProducer> PixelProducerPtr;
