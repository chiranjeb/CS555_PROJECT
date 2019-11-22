#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <queue>
#include <map>
#include <netinet/in.h>
#include "defines/defines_includes.hpp"
#include "framework/framework_includes.hpp"
#include "wiremsg/wire_msg.hpp"

class TCPIOSender;
class TCPIOReceiver;

class TCPIOConnection
{
public:
   /// Constructor
   TCPIOConnection(int socket, std::string clientIpAddress);
   TCPIOConnection();

   /// Return the remote address.
   std::string GetRemoteAddress()
   {
      return m_ip;
   }

   /// Allocate App tag
   int AllocateAppTag()
   {
       int tag = m_AppTagQ.front();
       m_AppTagQ.pop();
       return tag;
   }

   /// Free app tag
   void FreeAppTag(int appTag)
   {
       m_ClientRespRoutingMap.erase(appTag);
       m_AppTagQ.push(appTag);
   } 

   /// Send message
   void SendMsg(WireMsgPtr wireMsg, Listener *p_lis);

   /// Process received message
   void ProcessReceivedMsg(WireMsgPtr wireMsg);

   /// Connection is already established. Start the sender and receiver.
   void Start();

   /// Establish connection and then start the sender and receiver.
   bool Start(std::string& serverName, int port, bool retryUntillConnected);

   /// Returns the socket asscociated with this connection.
   int GetSocket()
   {
      return m_socket;
   }

private:
   /// Make connection
   int MakeConnection(std::string& server, int serverPort, bool retryUntilConnected);

   /// Remote ip connected to this machine.
   std::string m_ip;

   /// Asscociated socket with this TCP connection
   int m_socket;

   /// Associated sender thread with this TCP connection
   TCPIOSender  *m_pIOSender;

   /// Associated receiver thread with this TCP connection
   TCPIOReceiver  *m_pIOReceiver;

   /// App Tag Q
   std::queue<int> m_AppTagQ;
   std::map<int, Listener *> m_ClientRespRoutingMap;
   BlockingQueue<MsgPtr> m_SendQ;

   struct sockaddr_in m_server;
};
