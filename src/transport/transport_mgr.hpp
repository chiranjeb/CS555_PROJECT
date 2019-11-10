#pragma once
#include "tcp_server.hpp"

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
};
