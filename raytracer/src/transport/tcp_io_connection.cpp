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
    : m_Socket(socket), m_ip(clientIpAddress), m_SendQ(10)
{
    for (int index = 1; index < 50000; index++)
    {
        m_AppTagQ.push(index);
    }
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
    m_pIOReceiver = new TCPIOReceiver(this, m_Socket);
    m_pIOSender = new TCPIOSender(this, m_Socket, m_SendQ);

    // Start receiver and sender
    m_pIOReceiver->Start();
    m_pIOSender->Start();
}

void TCPIOConnection::SendMsg(WireMsgPtr wireMsg, Listener *p_lis)
{

    MsgPtr msg(nullptr);
    //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
    //                               " Sending TCP IP message(MsgId=" + wireMsg.get()->GetId() + ") to:" + m_Socket + "AppTag:" +  wireMsg.get()->GetAppTag());
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
