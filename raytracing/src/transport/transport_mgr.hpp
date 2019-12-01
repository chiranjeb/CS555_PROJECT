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
    void SaveConnection(std::string &unique_hostname, TCPIOConnectionPtr p_connection)
    {
        p_connection->SetUniqueHostName(unique_hostname);
        m_Connections[unique_hostname] = p_connection;
    }

    void RemoveConnection(std::string &unique_hostname)
    {
        m_Connections.erase(unique_hostname);    
    }

    /// Find connection
    TCPIOConnectionPtr FindConnection(std::string &unique_hostname);

    /// Create a TCP server
    void CreateTCPServer(int listeningPort, int listeningDepth, ListenerPtr serverResponseHandler);

    /// Process unsolicited message
    void ProcessUnsolicitedMsg(WireMsgPtr wireMsgPtr);

    /// Service new connection
    void ServiceNewConnection(TCPIOConnectionPtr p_connection);


    /// Establish connection to a server
    void EstablishNewConnection(std::string &serverIP, int serverPort, ListenerPtr p_lis, bool retryUntillConnected);

    /// notify connection exception
    void NotifyConnectionException(TCPIOConnectionPtr pConnection);

protected:
    TransportMgr();

    /// Host name
    std::string m_hostname;

    /// Asscociated TCP Server
    TCPIOServer *m_perver;

    /// Server command handler
    ListenerPtr m_lis;

    /// Connection list
    std::map<std::string, TCPIOConnectionPtr> m_Connections;
};
