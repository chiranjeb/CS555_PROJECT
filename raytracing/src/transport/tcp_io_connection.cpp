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
#include "tcp_io_send_recv.hpp"
#include "transport_mgr.hpp"
#include "transport_msgs.hpp"


TCPIOConnection::TCPIOConnection(int socket, std::string clientIpAddress)
    : m_socket(socket), m_ip(clientIpAddress), m_SendQ(10)
{
    for (int index = 1; index < 5000; index++)
    {
        m_AppTagQ.push(index);
    }
}

TCPIOConnection::TCPIOConnection() : m_SendQ(10)
{
    for (int index = 1; index < 5000; index++)
    {
        m_AppTagQ.push(index);
    }
}

////////////////////////////////////////////////////////////////////////////////////
///
/// Establish a connection initiated by client.
///
////////////////////////////////////////////////////////////////////////////////////
void TCPIOConnection::Start()
{
    m_pIOSender = nullptr;

    // Create the receiver and sender
    m_pIOReceiver = new TCPIOReceiver(this, m_socket);
    m_pIOSender = new TCPIOSender(this, m_socket, m_SendQ);

    // Start receiver and sender
    m_pIOReceiver->Start();
    m_pIOSender->Start();
}



void TCPIOConnection::RegisterNotification(int appTag, ListenerPtr plis)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    DEBUG_TRACE_TRANSPORT("TCPIOConnection::RegisterNotification - apptag: " << appTag <<", lis:" << plis);
    m_ClientRespRoutingMap.insert(std::pair<int, ListenerPtr>(appTag, plis));
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Send a message over a connection
///
/// @param [wireMsg] Pointer to the wire message
/// @param [p_lis] Listener which will be notified -
///       1. If a valid app tag is present in the message. Remote receiver is expected
///          to put the app tag back in the response message.
///       2. If requester is asking for an explicit response of the message being sent out.
///
///////////////////////////////////////////////////////////////////////////////////////////
void TCPIOConnection::SendMsg(WireMsgPtr wireMsg, ListenerPtr p_lis)
{
    MsgPtr msg(nullptr);
    RELEASE_TRACE("Sending TCP IP message: " << wireMsg.get()->GetId() << ", AppTag: " << wireMsg.get()->GetAppTag());
    if ((p_lis.get() != nullptr) && wireMsg.get()->ExpectingRecvRecvResponse())
    {
        RegisterNotification(wireMsg.get()->GetAppTag(), p_lis);
        m_pIOSender->SendMsg(MsgPtr(new TCPSendMsg(wireMsg, nullptr)));
    }
    else
    {
        m_pIOSender->SendMsg(MsgPtr(new TCPSendMsg(wireMsg, p_lis)));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Process a response message. There could be two cases here. Solicited and unsolicited
/// response. Solicited responses are routed based on the <apptag, lis> map. Unsolicited
/// responses are sent out to the unsolicited response handler registered in
/// TransportMgr::CreateTCPServer.
///
/// @param [wireMsgPtr] Pointer to the wire message
///
///////////////////////////////////////////////////////////////////////////////////////////
void TCPIOConnection::ProcessReceivedMsg(WireMsgPtr wireMsgPtr)
{
    wireMsgPtr->SetConnection(this);
    if (m_ClientRespRoutingMap.find(wireMsgPtr->GetAppTag()) != m_ClientRespRoutingMap.end())
    {
        DEBUG_TRACE_TRANSPORT("Received a solicited message: " << wireMsgPtr->GetId());
        // Give this to tag based handler
        m_ClientRespRoutingMap[wireMsgPtr->GetAppTag()]->Notify(wireMsgPtr);
    }
    else
    {
        DEBUG_TRACE_TRANSPORT("Received a unsolicited message: " << wireMsgPtr->GetId());
        TransportMgr::Instance().ProcessUnsolicitedMsg(this, wireMsgPtr);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Connect to a server and start connection
///
/// @param [serverName] Name of the server to connect
/// @param [port] Server listening port
/// @param [retryUntillConnected] Indicates whether to block current thread or not.
///
///
/////////////////////////////////////////////////////////////////////////////////////////
bool TCPIOConnection::Start(std::string& serverName, int port, bool retryUntillConnected)
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


/////////////////////////////////////////////////////////////////////////////////////////
/// Establishes connection
///
/// @param [serverName] Name of the server to connect
/// @param [port] Server listening port
/// @param [retryUntillConnected] Indicates whether to block current thread or not.
///
///
/////////////////////////////////////////////////////////////////////////////////////////
int TCPIOConnection::MakeConnection(std::string& server, int serverPort, bool retryUntilConnected)
{
    ErrorCode_t errorCode;
    while (true)
    {
        do
        {
            DEBUG_TRACE("Attempting to connect: " << server << ", port:" << serverPort);
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
