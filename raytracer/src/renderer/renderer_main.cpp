#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/transport_mgr.hpp"
#include "renderer/renderer_scheduler.hpp"

using namespace std;
class TestLis : public Listener
{
   virtual void Notify(shared_ptr<Msg> notificationMsg)
   {
      if (notificationMsg.get()->GetId() == MsgIdServerConstructResponse)
      {
         cerr << "Server Constructed successfully" << endl;
      }
   }
};

static const int SERVER_LISTENING_Q_DEPTH = 40;

int main()
{
   TestLis testLis;
   // TraceLogger.Instance().SetTraceLevel(TraceLogger.LEVEL_ALWAYS);

   // Start the Renderer Scheduler.
   RendererScheduler::Instance().Start();

   // Start a server
   TransportMgr::Instance().CreateTCPServer(8050, 40, *RendererScheduler::Instance().GetThrdListener());


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


