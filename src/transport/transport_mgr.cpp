#include "transport_mgr.hpp"

void TransportMgr::CreateTCPServer(int listeningPort, int listeningDepth, Listener &serverResponseHandler)
{
   // Create a IO Server thread
   m_perver = new TCPIOServer();

   // Construct the server
   m_perver->Construct(listeningPort, listeningDepth, serverResponseHandler);

   // Now start it
   m_perver->Start();
}

