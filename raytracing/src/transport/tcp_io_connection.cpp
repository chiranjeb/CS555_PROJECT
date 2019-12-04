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
    DEBUG_TRACE_TRANSPORT("TCPIOConnection Constructor");
    m_Closed = false;
    for (int index = 1; index < 5000; index++)
    {
        m_AppTagQ.push(index);
    }
}

TCPIOConnection::~TCPIOConnection()
{
    DEBUG_TRACE_TRANSPORT("TCPIOConnection Destructor");
}

TCPIOConnection::TCPIOConnection() : m_SendQ(10)
{
    DEBUG_TRACE_TRANSPORT("TCPIOConnection Constructor");
    m_Closed = false;
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
void TCPIOConnection::Start(TCPIOReceiver *pReceiver, TCPIOSender *pSender)
{
    // Create the receiver and sender
    m_pIOReceiver = pReceiver;
    m_pIOSender = pSender;

    // Start receiver and sender
    m_pIOReceiver->Start();
    m_pIOSender->Start();
}



void TCPIOConnection::RegisterNotification(int appTag, ListenerPtr plis)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    RegisterNotification_nolock(appTag, plis);
}


void TCPIOConnection::RegisterNotification_nolock(int appTag, ListenerPtr plis)
{
    DEBUG_TRACE_TRANSPORT("TCPIOConnection::RegisterNotification - apptag: " << appTag << ", lis:" << plis);
    m_ClientRespRoutingMap.insert(std::pair<int, ListenerPtr>(appTag, plis));
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Send a message over a connection
///
/// @param [msg] Pointer to the wire message
/// @param [p_lis] Listener which will be notified -
///       1. If a valid app tag is present in the message. Remote receiver is expected
///          to put the app tag back in the response message.
///       2. If requester is asking for an explicit response of the message being sent out.
///
///////////////////////////////////////////////////////////////////////////////////////////
void TCPIOConnection::SendMsg(MsgPtr msg, ListenerPtr p_lis)
{
    WireMsgPtr wireMsg = std::dynamic_pointer_cast<WireMsg>(msg);
    std::unique_lock<std::mutex> lck(m_Mutex);
    if (m_Closed == false)
    {
        DEBUG_TRACE_TRANSPORT("Sending TCP IP message: " << wireMsg->GetId() << ", AppTag: " << wireMsg->GetAppTag());
        if ((p_lis.get() != nullptr) && wireMsg->ExpectingRecvRecvResponse())
        {
            RegisterNotification_nolock(wireMsg->GetAppTag(), p_lis);
            m_pIOSender->SendMsg(std::make_shared<TCPSendMsg>(wireMsg, nullptr));
        }
        else
        {
            m_pIOSender->SendMsg(std::make_shared<TCPSendMsg>(wireMsg, p_lis));
        }
    }
    else if (p_lis != nullptr)
    {
        if (!wireMsg->ExpectingRecvRecvResponse())
        {
            TCPIOConnectionPtr connectionPtr = TransportMgr::Instance().FindConnection(m_UniqueHostName);
            p_lis->Notify(std::make_shared<TCPConnectionExceptionMsg>(connectionPtr));
        }
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
void TCPIOConnection::ProcessReceivedMsg(MsgPtr msg)
{
    WireMsgPtr wireMsgPtr = std::dynamic_pointer_cast<WireMsg>(msg);
    if (m_ClientRespRoutingMap.find(wireMsgPtr->GetAppTag()) != m_ClientRespRoutingMap.end())
    {
        DEBUG_TRACE_TRANSPORT("Received a solicited message: " << wireMsgPtr->GetId());
        // Give this to tag based handler
        m_ClientRespRoutingMap[wireMsgPtr->GetAppTag()]->Notify(wireMsgPtr);
    }
    else
    {
        DEBUG_TRACE_TRANSPORT("Received a unsolicited message: " << wireMsgPtr->GetId());
        TransportMgr::Instance().ProcessUnsolicitedMsg(wireMsgPtr);
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
            RELEASE_TRACE("Attempting to connect: " << server << ", port:" << serverPort);
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

/// Closes the connection
void TCPIOConnection::Close()
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    if (m_Closed == false)
    {
        /// close the socket.
        m_Closed = true;
        close(m_socket);
        // if we are initiating this, this will trigger the sender and receiver to see exception.
    }

    /// Request sender thread to come out
    if (m_pIOSender)
    {
        m_pIOSender->SendMsg(std::make_shared<Msg>(MsgIdTCPShutDownSender));
    }

    TCPIOConnectionPtr connectionPtr = TransportMgr::Instance().FindConnection(m_UniqueHostName);
    TransportMgr::Instance().RemoveConnection(m_UniqueHostName);
    DEBUG_TRACE("TCPIOConnectionPtr:" << std::hex << connectionPtr.get())
    /// Clean up all pending response stuff....
    for (auto client2RespRoutingMapIter = m_ClientRespRoutingMap.begin();
         client2RespRoutingMapIter != m_ClientRespRoutingMap.end();
         ++client2RespRoutingMapIter)
    {
        client2RespRoutingMapIter->second->Notify(std::make_shared<TCPConnectionExceptionMsg>(connectionPtr));
    }
    m_ClientRespRoutingMap.clear();
}


