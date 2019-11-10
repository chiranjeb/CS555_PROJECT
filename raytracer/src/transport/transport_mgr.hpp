#pragma once
#include "tcp_io_server.hpp"
#include "tcp_io_connection.hpp"
#include <list>

class TransportMgr
{
   TCPIOServer *m_perver;
    
    public:
    /** 
    * static method to get the instance of TCPMgr class
    */ 
    static TransportMgr& Instance()
    {
        static TransportMgr s_TransportMgr;
        return s_TransportMgr;
    }

    void CreateTCPServer(int listeningPort, int listeningDepth, Listener &serverResponseHandler);

    void ServiceNewConnection(TCPIOConnection *p_connection);

    Listener *m_lis;

    std::list<std::shared_ptr<TCPIOConnection>> m_Connections;
};
