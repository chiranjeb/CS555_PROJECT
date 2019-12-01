#include "transport_mgr.hpp"
#include "tcp_io_connection.hpp"
#include "transport_msgs.hpp"
#include "tcp_io_send_recv.hpp"

TransportMgr::TransportMgr()
{
    char hostbuffer[128];
    memset(hostbuffer, 0, sizeof(hostbuffer));
    gethostname(hostbuffer, sizeof(hostbuffer));
    m_hostname = std::string(hostbuffer);
}

void TransportMgr::CreateTCPServer(int listeningPort, int listeningDepth, ListenerPtr serverResponseHandler)
{
    m_lis = serverResponseHandler;

    /// Create a IO Server thread
    m_perver = new TCPIOServer();

    /// Construct the server
    ErrorCode_t errorCode;
    if ((errorCode = m_perver->Construct(listeningPort, listeningDepth)) == STATUS_SUCCESS)
    {
        /// Now start it.
        m_perver->Start();
    }

    /// Finally notify the cmd processor.
    m_lis->Notify(std::shared_ptr<Msg>(new TCPServerConstructStatusMsg(errorCode, m_perver->GetPort())));
}

TCPIOConnectionPtr TransportMgr::FindConnection(std::string& unique_hostname)
{
    auto entry = m_Connections.find(unique_hostname);
    if (entry == m_Connections.end())
    {
        return nullptr;
    }
    else
    {
        return entry->second;
    }
}

void TransportMgr::ProcessUnsolicitedMsg(WireMsgPtr wireMsgPtr)
{
    m_lis->Notify(MsgPtr(new TCPRecvMsg(wireMsgPtr->GetConnection(), wireMsgPtr)));
}


void TransportMgr::ServiceNewConnection(TCPIOConnectionPtr p_connection)
{
    p_connection->Start(new TCPIOReceiver(p_connection, p_connection->GetSocket()),
                        new TCPIOSender(p_connection, p_connection->GetSocket(), p_connection->GetSendQ()));
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

void TransportMgr::EstablishNewConnection(std::string& serverIP, int serverPort, ListenerPtr p_lis, bool retryUntillConnected)
{
    auto entry = m_Connections.find(UniqueServerId(serverIP, serverPort).toString());
    if (entry == m_Connections.end())
    {
        TCPIOConnectionPtr ioConnection = std::make_shared<TCPIOConnection>();
        if (ioConnection->MakeConnection(serverIP, serverPort, retryUntillConnected) != -1)
        {
            ioConnection->Start(new TCPIOReceiver(ioConnection, ioConnection->GetSocket()),
                                new TCPIOSender(ioConnection, ioConnection->GetSocket(), ioConnection->GetSendQ()));
            p_lis->Notify(std::make_shared<TCPConnectionEstablishRespMsg>(STATUS_SUCCESS, ioConnection, serverIP, serverPort));
        }
        else
        {
            p_lis->Notify(std::make_shared<TCPConnectionEstablishRespMsg>(ERR_TRANSPORT_CONNECTION_FAIL_TO_ESTABLISH_CONNECTION,
                                                                          ioConnection, serverIP, serverPort));
        }
    }
    else
    {
        p_lis->Notify(std::make_shared<TCPConnectionEstablishRespMsg>(STATUS_SUCCESS, entry->second, serverIP, serverPort));
    }
}



/// notify connection exception
void TransportMgr::NotifyConnectionException(TCPIOConnectionPtr pConnection)
{
    if (m_lis.get() != nullptr)
    {
        /// Notify the upper layer about connection exception. So that it can clean things up
        m_lis->Notify(std::make_shared<TCPConnectionExceptionMsg>(pConnection));
    }
}


