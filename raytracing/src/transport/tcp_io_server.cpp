#include "tcp_io_server.hpp"
#include "transport_mgr.hpp"
using namespace std;

TCPIOConnection* TCPIOServer::AcceptConnection()
{
    socklen_t sosize  = sizeof(m_clientAddress);


    int file_descriptor = accept(m_sockfd, (struct sockaddr *)&m_clientAddress, &sosize);
    if (file_descriptor == -1)
    {
        return nullptr;
    }

    // Create a connection.
    return new TCPIOConnection(file_descriptor, inet_ntoa(m_clientAddress.sin_addr));
}


/*
 * Server thread 
 *  
 */
void TCPIOServer::Run()
{
    while (true)
    {
        DEBUG_TRACE("TCPIOServer::Wait for a new connection");
        // Accept a new connection
        TCPIOConnection *p_connection = AcceptConnection();

        // Punt it up and request to service new connection
        TransportMgr::Instance().ServiceNewConnection(p_connection);
    }
}



ErrorCode_t TCPIOServer::Construct(int port, int clientsQueueSize)
{
    ErrorCode_t errorCode = STATUS_SUCCESS;
    do
    {
        m_sockfd = 0;

        m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_sockfd == -1)
        {
            errorCode = ERR_TRANSPORT_SOCKET_CREATION_FAILED;
            break;
        }

        // set socket for reuse (otherwise might have to wait 4 minutes every time socket is closed)
        int option = 1;
        setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        memset(&m_serverAddress, 0, sizeof(m_serverAddress));
        m_serverAddress.sin_family = AF_INET;
        m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        m_serverAddress.sin_port = htons(port);

        int bindSuccess = bind(m_sockfd, (struct sockaddr *)&m_serverAddress, sizeof(m_serverAddress));
        if (bindSuccess == -1)
        {
            // bind failed
            errorCode =  ERR_TRANSPORT_BIND_FAILED;
            break;
        }
        int listenSuccess = listen(m_sockfd, clientsQueueSize);
        if (listenSuccess == -1)
        {
            // listen failed
            errorCode =  ERR_TRANSPORT_LISTEN_FAILED;
            break;
        }
    }while (0);

    if (errorCode == STATUS_SUCCESS && (m_listeningPort == 0))
    {
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(m_sockfd, (struct sockaddr *)&sin, &len) != -1)
        {
            m_listeningPort = ntohs(sin.sin_port);
        }
        else
        {
            errorCode =  ERR_TRANSPORT_FAILED_TO_LOCATE_SERVER_PORT;
        }
    }
    return errorCode;
}
