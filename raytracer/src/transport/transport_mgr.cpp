#include "transport_mgr.hpp"
#include "tcp_io_connection.hpp"
#include "transport_msgs.hpp"

void TransportMgr::CreateTCPServer(int listeningPort, int listeningDepth, Listener &serverResponseHandler)
{
    m_lis = &serverResponseHandler;

    // Create a IO Server thread
    m_perver = new TCPIOServer();

    // Construct the server
    ErrorCode_t errorCode;
    if ((errorCode = m_perver->Construct(listeningPort, listeningDepth)) == STATUS_SUCCESS)
    {
        // Now start it.
        m_perver->Start();
    }

    // Finally notify the cmd processor.
    m_lis->Notify(std::shared_ptr<Msg>(new StatusMsg(MsgIdServerConstructResponse, errorCode)));
}


void TransportMgr::ProcessUnsolicitedMsg(TCPIOConnection *p_connection, WireMsgPtr wireMsgPtr)
{
    //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,  "Received WireMsg(MsgId:" + wireMsgPtr.get()->GetId()+ ")");
    m_lis->Notify(MsgPtr(new TCPRecvMsg(p_connection, wireMsgPtr)));
}


void TransportMgr::ServiceNewConnection(TCPIOConnection *p_connection)
{
    m_Connections.push_back(p_connection);
    p_connection->Start();
}


