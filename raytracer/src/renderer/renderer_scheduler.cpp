#include <iostream>
#include "renderer_scheduler.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"

void RendererScheduler::Run()
{
    RELEASE_TRACE("Started Renderer Scheduler thread");
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

void RendererScheduler::OnTCPRecvMsg(MsgPtr msg)
{
    DEBUG_TRACE("On TCP Recv Message");
    TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr.get()->GetId())
    {
        case MsgIdWorkerRegistrationRequest:
        {
            WorkerRegistrationMsg *pWireMsg = static_cast<WorkerRegistrationMsg *>(wireMsgPtr.get());
            pWireMsg->Dump();
            WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
            reigstrationRespMsgPtr.get()->SetAppTag(pWireMsg->GetAppTag());
            //reigstrationMsgPtr.get()->SetAppTag(m_p_TCPIOConnection->AllocateAppTag());
            pWireMsg->GetConnection()->SendMsg(reigstrationRespMsgPtr, nullptr);
            break;
        }
        default:
            break;
    }
}


