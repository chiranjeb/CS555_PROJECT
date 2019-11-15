#include <iostream>
#include "master_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"

void MasterScheduler::Run()
{
    RELEASE_TRACE("Started Master Scheduler thread");
    while (1)
    {
        MsgQEntry msgQEntry = TakeNext();
        MsgPtr msgPtr = msgQEntry.m_Msg;
        if (msgQEntry.m_Cmd.get() != nullptr)
        {
            msgQEntry.m_Cmd.get()->ProcessMsg(msgQEntry.m_Msg);
        }
        else
        {
            switch (msgQEntry.m_Msg.get()->GetId())
            {
                case MsgIdServerConstructResponse:
                    DEBUG_TRACE("Server Started");
                    break;

                case MsgIdTCPRecv:
                    OnTCPRecvMsg(msgQEntry.m_Msg);
                    break;

                default:
                    break;
            }
        }
    }
}

void MasterScheduler::OnTCPRecvMsg(MsgPtr msg)
{
    DEBUG_TRACE("On TCP Recv Message");
    TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr.get()->GetId())
    {
        case MsgIdWorkerRegistrationRequest:
        {
            OnWorkerRegistrationRequest(wireMsgPtr);
            break;
        }
        default:
            break;
    }
}

void MasterScheduler::OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr)
{
    WorkerRegistrationMsg *pWireMsg = static_cast<WorkerRegistrationMsg *>(wireMsgPtr.get());
    pWireMsg->Dump();

    /// Register the host name
    std::string unique_hostname = UniqueServerId(pWireMsg->m_hostname, pWireMsg->m_Port).toString();
    m_workerlist.push_back(unique_hostname);
    TransportMgr::Instance().SaveConnection(unique_hostname, pWireMsg->GetConnection());
    WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
    reigstrationRespMsgPtr.get()->SetAppTag(pWireMsg->GetAppTag());

    /// We are not expecting any response back. So, pass a nullptr.
    pWireMsg->GetConnection()->SendMsg(reigstrationRespMsgPtr, nullptr);

    /// Dump the worker lists
    DEBUG_TRACE("worker list: " << m_workerlist.size());
    for ( std::list<std::string>::iterator iter = m_workerlist.begin(); iter!= m_workerlist.end(); iter++)
    {
        DEBUG_TRACE("worker: " << *iter);
    }
}


