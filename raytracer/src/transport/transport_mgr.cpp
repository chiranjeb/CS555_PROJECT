#include "transport_mgr.hpp"
#include "tcp_io_connection.hpp"
#include "transport_msgs.hpp"

TransportMgr::TransportMgr()
{
    char hostbuffer[128]; 
    memset(hostbuffer, 0, sizeof(hostbuffer));
    gethostname(hostbuffer, sizeof(hostbuffer)); 
    m_hostname = std::string(hostbuffer);
}

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
   m_lis->Notify(std::shared_ptr<Msg>(new TCPServerConstructStatusMsg(errorCode, m_perver->GetPort())));
}


void TransportMgr::ProcessUnsolicitedMsg(TCPIOConnection *p_connection, WireMsgPtr wireMsgPtr)
{
   m_lis->Notify(MsgPtr(new TCPRecvMsg(p_connection, wireMsgPtr)));
}


void TransportMgr::ServiceNewConnection(TCPIOConnection *p_connection)
{
   m_Connections["New"]=p_connection; // todo change
   p_connection->Start();
}

//////////////////////////////////////////////////////////////////////////////////////////////
/// Connect to a Server .
///
/// @param serverIP : Server address to connect to.
/// @param serverPort:  port Server port to connect to .
/// @param p_lis : ListenerIf which will be notified with connection
///                establishment status message.
/// @param retryUntillConnected : Retry untill connected or not
///
//////////////////////////////////////////////////////////////////////////////////////////////

void TransportMgr::EstablishNewConnection(std::string &serverIP, int serverPort, Listener *p_lis, bool retryUntillConnected)
{
   auto entry = m_Connections.find(serverIP + std::to_string(serverPort));
   if (entry == m_Connections.end())
   {
      TCPIOConnection *ioConnection = new TCPIOConnection();
      if (ioConnection->Start(serverIP, serverPort, retryUntillConnected))
      {
         p_lis->Notify(MsgPtr(new TCPConnectionEstablishRespMsg(STATUS_SUCCESS, ioConnection, serverIP, serverPort)));
      }
      else
      {
         p_lis->Notify(MsgPtr(new TCPConnectionEstablishRespMsg(ERR_TRANSPORT_CONNECTION_FAIL_TO_ESTABLISH_CONNECTION, ioConnection, serverIP, serverPort)));
      }
   }
   else
   {
      p_lis->Notify(MsgPtr(new TCPConnectionEstablishRespMsg(STATUS_SUCCESS, entry->second, serverIP, serverPort)));
   }
}


