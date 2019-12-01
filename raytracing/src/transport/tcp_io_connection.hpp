#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <queue>
#include <map>
#include <netinet/in.h>
#include "defines/defines_includes.hpp"
#include "framework/framework_includes.hpp"
#include <memory>

class TCPIOSender;
class TCPIOReceiver;

class TCPIOConnection
{
public:
   /// Constructor
   TCPIOConnection(int socket, std::string clientIpAddress);
   TCPIOConnection();
   ~TCPIOConnection();

   /// Return the remote address.
   std::string GetRemoteAddress()
   {
      return m_ip;
   }

   /// Allocate App tag
   int AllocateAppTag()
   {
       std::unique_lock<std::mutex> lck(m_Mutex);
       int tag = m_AppTagQ.front();
       m_AppTagQ.pop();
       return tag;
   }

   /// Free app tag
   void FreeAppTag(int appTag)
   {
       std::unique_lock<std::mutex> lck(m_Mutex);
       m_ClientRespRoutingMap.erase(appTag);
       m_AppTagQ.push(appTag);
   } 

   /// Regiser for notification
   void RegisterNotification(int appTag, ListenerPtr p_lis);

   /// Send message
   void SendMsg(MsgPtr wireMsg, ListenerPtr p_lis);

   /// Process received message
   void ProcessReceivedMsg(MsgPtr wireMsg);

   /// Connection is already established. Start the sender and receiver.
   void Start(TCPIOReceiver* pReceiver, TCPIOSender* pSender);

   /// Establish connection and then start the sender and receiver.
   bool Start(std::string& serverName, int port, bool retryUntillConnected);

   /// Returns the socket asscociated with this connection.
   int GetSocket()
   {
      return m_socket;
   }

   /// Get remote hostname.
   std::string& GetUniqueHostName()
   {
       return m_UniqueHostName;
   }

   /// Set remote hostname.
   void SetUniqueHostName(std::string hostname)
   {
       m_UniqueHostName = hostname;
   }

   /// Close the connection.
   void Close();

   /// Notify connection exception
   void NotifyException();

   BlockingQueue<MsgPtr>& GetSendQ()
   {
       return m_SendQ;
   }

   /// Make connection
   int MakeConnection(std::string& server, int serverPort, bool retryUntilConnected);

private:
   void RegisterNotification_nolock(int appTag, ListenerPtr plis);

   /// Remote ip connected to this machine.
   std::string m_ip, m_UniqueHostName;

   /// Asscociated socket with this TCP connection
   int m_socket;

   /// Associated sender thread with this TCP connection
   TCPIOSender  *m_pIOSender;

   /// Associated receiver thread with this TCP connection
   TCPIOReceiver  *m_pIOReceiver;

   /// App Tag Q
   std::queue<int> m_AppTagQ;
   std::map<int, ListenerPtr> m_ClientRespRoutingMap;
   BlockingQueue<MsgPtr> m_SendQ;

   std::mutex m_Mutex;
   struct sockaddr_in m_server;
   bool m_Closed;
};

typedef std::shared_ptr<TCPIOConnection> TCPIOConnectionPtr;
