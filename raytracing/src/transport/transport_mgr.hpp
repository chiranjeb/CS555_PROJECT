#pragma once
#include <list>
#include "tcp_io_server.hpp"
#include "tcp_io_connection.hpp"
#include "wiremsg/wire_msg.hpp"

struct UniqueServerId
{
    UniqueServerId(std::string servername, int port):m_servername(servername), m_port(port)
    {
    }

    std::string toString()
    {
        return m_servername + ":" + std::to_string(m_port);
    }

    int m_port;
    std::string m_servername;
};

class TransportMgr
{
public:
    /// static method to get the instance of TCPMgr class
    static TransportMgr &Instance()
    {
        static TransportMgr s_TransportMgr;
        return s_TransportMgr;
    }

    /// Return hostname
    std::string& MyName()
    {
        return m_hostname;
    }

    /// Save a connection.
    void SaveConnection(std::string &unique_hostname, TCPIOConnection *p_connection)
    {
        m_Connections[unique_hostname] = p_connection;
    }

    /// Find connection
    TCPIOConnection* FindConnection(std::string &unique_hostname);

    /// Create a TCP server
    void CreateTCPServer(int listeningPort, int listeningDepth, Listener &serverResponseHandler);

    /// Process unsolicited message
    void ProcessUnsolicitedMsg(TCPIOConnection *p_connection, WireMsgPtr wireMsgPtr);

    /// Service new connection
    void ServiceNewConnection(TCPIOConnection *p_connection);


    /// Establish connection to a server
    void EstablishNewConnection(std::string &serverIP, int serverPort, Listener *p_lis, bool retryUntillConnected);

protected:
    TransportMgr();

    /// Host name
    std::string m_hostname;

    /// Asscociated TCP Server
    TCPIOServer *m_perver;

    /// Server command handler
    Listener *m_lis;

    /// Connection list
    std::map<std::string, TCPIOConnection *> m_Connections;
};
