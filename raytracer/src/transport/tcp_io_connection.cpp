#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <vector>
#include <errno.h>
#include <thread>
#include "tcp_io_connection.hpp"
#include "tcp_io_receiver.hpp"
#include "tcp_io_sender.hpp"
#include "transport_mgr.hpp"
#include "transport_msgs.hpp"


TCPIOConnection::TCPIOConnection(int socket, std::string clientIpAddress)
   : m_socket(socket), m_ip(clientIpAddress), m_SendQ(10)
{
   for (int index = 1; index < 50000; index++)
   {
      m_AppTagQ.push(index);
   }
}

TCPIOConnection::TCPIOConnection(): m_SendQ(10)
{
}

/** 
* Establish a connection initiated by client. 
* @throw IOException on IO error.
* @param socket client socket 
*/
void TCPIOConnection::Start()
{
   //m_SendQ = nullptr;
   m_pIOSender = nullptr;

   //If a TCP connection is used using socket. We will create a recvQ only and let
   //upper layer chose whether to create a send thread or not.
   //m_pIOReceiver = new TCPIOReceiver(m_Socket, this);
   m_pIOReceiver = new TCPIOReceiver(this, m_socket);
   m_pIOSender = new TCPIOSender(this, m_socket, m_SendQ);

   // Start receiver and sender
   m_pIOReceiver->Start();
   m_pIOSender->Start();
}

void TCPIOConnection::SendMsg(WireMsgPtr wireMsg, Listener *p_lis)
{

   MsgPtr msg(nullptr);
   //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
   //                               " Sending TCP IP message(MsgId=" + wireMsg.get()->GetId() + ") to:" + m_Socket + "AppTag:" +  wireMsg.get()->GetAppTag());
   std::cerr <<  "Sending TCP IP message" << wireMsg.get()->GetId() << ", AppTag: " << wireMsg.get()->GetAppTag() << std::endl;
   if ((p_lis != nullptr) && wireMsg.get()->ExpectingRecvRecvResponse())
   {
      m_ClientRespRoutingMap.insert(std::pair<int, Listener *>(wireMsg.get()->GetAppTag(), p_lis));
      m_pIOSender->SendMsg(MsgPtr(new TCPSendMsg(wireMsg, nullptr)));
   }
   else
   {
      m_pIOSender->SendMsg(MsgPtr(new TCPSendMsg(wireMsg, p_lis)));
   }
}

void TCPIOConnection::ProcessReceivedMsg(WireMsgPtr wireMsgPtr)
{
   WireMsg *pWireMsg = wireMsgPtr.get();
   pWireMsg->SetConnection(this);
   std::cerr << "Received a wire message:" << wireMsgPtr.get()->GetId() << std::endl;
   if (m_ClientRespRoutingMap.find(pWireMsg->GetAppTag()) != m_ClientRespRoutingMap.end())
   {
      // Give this to tag based handler
      // TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT, "Received WireMsg(MsgId:" + wireMsg.GetId() + ")" + "AppTag:" + wireMsg.GetAppTag());
      m_ClientRespRoutingMap[pWireMsg->GetAppTag()]->Notify(wireMsgPtr);
   }
   else
   {
      TransportMgr::Instance().ProcessUnsolicitedMsg(this, wireMsgPtr);
   }
}


bool TCPIOConnection::Start(std::string &serverName, int port, bool retryUntillConnected)
{
   m_socket = MakeConnection(serverName, port, retryUntillConnected);
   if (m_socket != -1)
   {
      Start();
      return true;
   }
   else
   {
      return false;
   }
}

int TCPIOConnection::MakeConnection(std::string& server, int serverPort, bool retryUntilConnected)
{
   ErrorCode_t errorCode;
   while (true)
   {
      do
      {
         std::cerr <<  "Attempting to connect: " << server << "port:" << serverPort << std::endl;
         //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
         //                                " Attempting to connect:" + serverIP + "port:" + serverPort);
         m_socket = socket(AF_INET, SOCK_STREAM, 0);
         if (m_socket == -1) //socket failed
         {
            errorCode = ERR_TRANSPORT_CONNECTION_FAILED_TO_CREATE_SOCKET;
            break;
         }

         int inetSuccess = inet_aton(server.c_str(), &m_server.sin_addr);

         if (!inetSuccess) // inet_addr failed to parse address
         {
            // if hostname is not in IP strings and dots format, try resolve it
            struct hostent *host;
            struct in_addr **addrList;
            if ((host = gethostbyname(server.c_str())) == NULL)
            {
               errorCode = ERR_TRANSPORT_CONNECTION_FAIL_TO_PARSER_HOST_NAME;
               break;
            }
            addrList = (struct in_addr **)host->h_addr_list;
            m_server.sin_addr = *addrList[0];
         }
         m_server.sin_family = AF_INET;
         m_server.sin_port = htons(serverPort);

         int connectRet = connect(m_socket, (struct sockaddr *)&m_server, sizeof(m_server));
         if (connectRet == -1)
         {
            errorCode = ERR_TRANSPORT_CONNECTION_FAIL_TO_ESTABLISH_CONNECTION;
            break;
         }
         errorCode = STATUS_SUCCESS;
      }
      while (0);


      if (errorCode == STATUS_SUCCESS)
      {
         return m_socket;
      }
      else if (retryUntilConnected)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      }
   }
   return -1;
}
