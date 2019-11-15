#include <iostream>
#include "worker.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"

void Worker::Run()
{
    RELEASE_TRACE("Started Worker thread");
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
                {
                    OnCreateServerResponse(msgQEntry.m_Msg);
                    break;
                }
                case MsgIdConnectionEstablishmentResponse:
                {
                    OnConnectionEstablishmentResponseMsg(msgQEntry.m_Msg);
                    break;
                }
                case MsgIdWorkerRegistrationResponse:
                {
                    OnWorkerRegistrationRespMsg(msgQEntry.m_Msg);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void Worker::OnCreateServerResponse(MsgPtr msg)
{
    DEBUG_TRACE("Worker::OnCreateServerResponse");
    TCPServerConstructStatusMsg *p_responseMsg =  static_cast<TCPServerConstructStatusMsg *>(msg.get());
    m_listening_port  = p_responseMsg->GetPort();

    // Let's establish a connection
    TransportMgr::Instance().EstablishNewConnection(m_master_address, m_master_port, GetThrdListener(), true);
}

void Worker::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
    RELEASE_TRACE("Successfully established connection");
    TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
    m_p_ConnectionToMaster = p_responseMsg->GetConnection();
    std::string hostname = TransportMgr::Instance().MyName();
    WorkerRegistrationMsgPtr reigstrationMsgPtr =  WorkerRegistrationMsgPtr(new WorkerRegistrationMsg(hostname, m_listening_port));
    reigstrationMsgPtr.get()->SetAppTag(m_p_ConnectionToMaster->AllocateAppTag());
    m_p_ConnectionToMaster->SendMsg(reigstrationMsgPtr, GetThrdListener());
}

void Worker::OnWorkerRegistrationRespMsg(MsgPtr msg)
{
    RELEASE_TRACE("Worker::OnWorkerRegistrationRespMsg");
    WorkerRegistrationRespMsg *p_responseMsg =  static_cast<WorkerRegistrationRespMsg *>(msg.get());
    p_responseMsg->Dump();
    p_responseMsg->GetConnection()->FreeAppTag(p_responseMsg->GetAppTag());
}
