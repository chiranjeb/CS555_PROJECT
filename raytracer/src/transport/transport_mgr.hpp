#pragma once
#include <list>
#include "tcp_io_server.hpp"
#include "tcp_io_connection.hpp"
#include "wiremsg/wire_msg.hpp"

class TransportMgr
{
public:
    /// static method to get the instance of TCPMgr class
    static TransportMgr &Instance()
    {
        static TransportMgr s_TransportMgr;
        return s_TransportMgr;
    }

    /// Create a TCP server
    void CreateTCPServer(int listeningPort, int listeningDepth, Listener &serverResponseHandler);

    /// Process unsolicited message
    void ProcessUnsolicitedMsg(TCPIOConnection *p_connection, WireMsgPtr wireMsgPtr);

    /// Service new connection
    void ServiceNewConnection(TCPIOConnection *p_connection);

protected:
    /// Asscociated TCP Server
    TCPIOServer *m_perver;

    /// Server command handler
    Listener *m_lis;

    /// Connection list
    std::list<TCPIOConnection *> m_Connections;
};
