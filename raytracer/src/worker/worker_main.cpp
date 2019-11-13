#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "worker.hpp"

using namespace std;

int main()
{
   // TraceLogger.Instance().SetTraceLevel(TraceLogger.LEVEL_ALWAYS);

   // Start the Renderer Scheduler.
   Worker::Instance().Start();

   std::string renderer_address = "albany";


   // Establish connection with the renderer.
   TransportMgr::Instance().EstablishNewConnection(renderer_address,  8050,  Worker::Instance().GetThrdListener(), true);


   while (1)
   {

   }
   // Submit jobs
   //RendererCli.Instance().Start();




   /*
   // Start the threads.
   //m_CmdProcessorThrd.start();

   // Create a TCP server and find the port where we are listnening
   //EvtListener evtListener = new EvtListener();
   //TCPMgr.Instance().CreateServer(m_peerNodeServerPort, 
                                  PEER_SERVER_LISTENING_Q_DEPTH, 
                                  true, 
                                  m_CmdProcessorThrd.GetThrdListener(), 
                                  evtListener);
   evtListener.Wait();

   m_MyAddress = UniqueTCPServerId.GetLocalHostName();
   m_peerNodeServerPort = ((TCPServerStatusMsg)evtListener.GetNotificationMsg()).GetListeningPort();


   // Establish connection with discovery node and let the store keeper thread talk to discovery node.
   TCPMgr.Instance().EstablishNewConnection(m_discoveryNodeIPAddress, 
                                            m_discoveryNodeListeningPort, 
                                            PEER_TO_DISCOVERY_SEND_Q_DEPTH,
                                            m_CmdProcessorThrd.GetThrdListener(), true);

   // Put the main thread to sleep.
   while (true)
   {
       // Start the cli
       PeerNodeCli.Run();
   }


   //Create 
     cout << "Hello Renderer Main" << endl;
     Transport();
     int i;
     cin >> i;
    return 0;
        */
}

