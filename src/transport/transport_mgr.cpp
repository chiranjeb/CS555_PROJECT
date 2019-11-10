#include "transport_mgr.hpp"
#include "tcp_io_connection.hpp"

void TransportMgr::CreateTCPServer(int listeningPort, int listeningDepth, Listener& serverResponseHandler)
{
   m_lis = &serverResponseHandler;

   // Create a IO Server thread
   m_perver = new TCPIOServer();

   // Construct the server
   ErrorCode_t errorCode;
   if ((errorCode = m_perver->Construct(listeningPort, listeningDepth)) == STATUS_SUCCESS)
   {
      // Now start it.
      m_perver->Start();
   }

   // Finally notify the cmd processor.
   m_lis->Notify(std::shared_ptr<Msg>(new StatusMsg(MsgIdServerConstructResponse, errorCode)));
}



void TransportMgr::ServiceNewConnection(TCPIOConnection *p_connection)
{
   m_Connections.push_back(std::shared_ptr<TCPIOConnection>(p_connection));
   p_connection->Start();
}

