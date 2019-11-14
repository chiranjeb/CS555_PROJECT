#include <sys/types.h>
#include <sys/socket.h>
#include "tcp_io_sender.hpp"
#include "transport_msgs.hpp"

void TCPIOSender::Run()
{
    std::cerr << "Started TCPIOSender::Run thread" << std::endl;
    while (1)
    {
        std::cerr << "Running TCPIOSender::Run thread" << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        m_State = STATE_RUNNING;

        //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
        //                               "Starting");

        m_Stop = false;

        while (m_Stop == false)
        {
            //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT, "Wait for Processing Message");
            //Wait for a message
            MsgPtr msgPtr = m_SendQ.Take();


            //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
            //                               " Processing Message(MsgId:" + msgQEntry.m_Msg.GetId() + ")");

            switch (msgPtr.get()->GetId())
            {
                case MsgIdTCPSend:
                    OnTCPSendMsg(msgPtr);
                    break;

                case MsgIdTCPShutDownSender:
                    m_State = STATE_EXCEPTION;
                    m_Stop = true;
                    break;

                default:
                    //TraceLogger.Instance().Println(TraceLogger.LEVEL_WARNING, TraceLogger.MODULE_TRANSPORT,
                    //                               " Received an unknown Message(MsgId:" + msgQEntry.m_Msg.GetId() + ")");
                    break;
            }
            //Go back and see if there is anything to send out.
        }

        // Mark everything as faulted which will send notifications to the caller.
        //m_p_connection.NotifyConnectionFault();
    }
}



void TCPIOSender::OnTCPSendMsg(MsgPtr requestMsgPtr)
{
    ErrorCode_t errorCode = STATUS_SUCCESS;
    if (m_State == STATE_RUNNING)
    {
        MsgPtr msgPtr = requestMsgPtr;
        TCPSendMsg *tcpSendMsg = dynamic_cast<TCPSendMsg *>(msgPtr.get());

        //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_TRANSPORT,
        //                               " Sending WireMsg(MsgId:" + requestMsg.GetWireMsg().GetId()+ ")");


        std::pair<char *, int> buffer = tcpSendMsg->GetWireMsg().get()->GetPackedBytes(&m_MsgBuffer[0], MAX_MSG_BUFFER_SIZE_IN_BYTES);

        std::cerr <<  "Sending wire message Message(MsgId:" << msgPtr.get()->GetId() << ")" << "buffer_lebgth:" << buffer.second << std::endl;


        m_MsgLengthBuff [0] = buffer.second & 0x000000FF;
        m_MsgLengthBuff [1] = (buffer.second & 0x0000FF00) >> 8;
        m_MsgLengthBuff [2] = (buffer.second & 0x00FF0000) >> 16;
        m_MsgLengthBuff [3] = (buffer.second & 0xFF000000) >> 24;

        int numBytesSent = send(m_socket, m_MsgLengthBuff, sizeof(m_MsgLengthBuff), 0);
        if (numBytesSent < 0)
        {
            // connection closed. We need to clean things up
            errorCode = ERR_TRANSPORT_CONNECTION_CLOSED;
        }
        else if (numBytesSent < buffer.second)
        {
            // not all bytes were sent
            errorCode = ERR_TRANSPORT_FAIL_TO_XMIT_ALL_DATA;
        }
        else
        {
            errorCode = STATUS_SUCCESS;
            std::cerr <<  "Successfully sent the data length" << buffer.second <<  std::endl;
        }


        numBytesSent = send(m_socket, buffer.first, buffer.second, 0);
        if (numBytesSent < 0)
        {
            // connection closed. We need to clean things up
            errorCode = ERR_TRANSPORT_CONNECTION_CLOSED;
        }
        else if (numBytesSent < buffer.second)
        {
            // not all bytes were sent
            errorCode = ERR_TRANSPORT_FAIL_TO_XMIT_ALL_DATA;
        }
        else
        {
            errorCode = STATUS_SUCCESS;
        }

        if (tcpSendMsg->GetLis() != nullptr)
        {
            tcpSendMsg->GetLis()->Notify(MsgPtr(new TCPSendStatusMsg(tcpSendMsg->GetWireMsg().get()->GetId(), errorCode)));
        }

        // We are done this message. Just free the shared pointer.
        msgPtr.reset();
    }
    else
    {
        // We are in exception state. Just drop the message.
    }
}
