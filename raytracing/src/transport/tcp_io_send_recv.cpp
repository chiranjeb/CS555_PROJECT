#include <sys/types.h>
#include <sys/socket.h>
#include "transport_msgs.hpp"
#include "tcp_io_send_recv.hpp"
#include "tcp_io_connection.hpp"
#include "wiremsg/wire_msg_factory.hpp"

void TCPIOSender::Run()
{
    RELEASE_TRACE("Started TCPIOSender::Run thread.");
    while (1)
    {
        DEBUG_TRACE("Running TCPIOSender::Run thread.");
        m_State = STATE_RUNNING;
        m_Stop = false;
        while (m_Stop == false)
        {
            //Wait for a message
            MsgPtr msgPtr = m_SendQ.Take();
            switch (msgPtr.get()->GetId())
            {
                case MsgIdTCPSend:
                    OnTCPSendMsg(msgPtr);
                    break;

                case MsgIdTCPShutDownSender:
                    DEBUG_TRACE("Released a shutdown request.");
                    m_State = STATE_EXCEPTION;
                    m_Stop = true;
                    break;

                default:
                    DEBUG_TRACE("Received an unknown Message, MsgId: " << msgPtr.get()->GetId());
                    break;
            }
            //Go back and see if there is anything to send out.
        }
        ///@@@ Need to handle connection fault
    }
}

ErrorCode_t TCPIOSender::SendData(char *data, int size)
{
    ErrorCode_t errorCode;
    int numBytesSent = send(m_socket, data, size, 0);
    if (numBytesSent < 0)
    {
        // connection closed. We need to clean things up
        errorCode = ERR_TRANSPORT_CONNECTION_CLOSED;
    }
    else if (numBytesSent < size)
    {
        // not all bytes were sent
        errorCode = ERR_TRANSPORT_FAIL_TO_XMIT_ALL_DATA;
    }
    else
    {
        errorCode = STATUS_SUCCESS;
        //DEBUG_TRACE("Successfully sent some data: " << size << "(bytes)");
    }
    return errorCode;
}

void TCPIOSender::OnTCPSendMsg(MsgPtr requestMsgPtr)
{
    ErrorCode_t errorCode = STATUS_SUCCESS;
    if (m_State == STATE_RUNNING)
    {
        MsgPtr msgPtr = requestMsgPtr;
        TCPSendMsg *tcpSendMsg = static_cast<TCPSendMsg *>(msgPtr.get());

        // We may need to revisit this when are done identifying all the messages.
        std::pair<char *, int> buffer = tcpSendMsg->GetWireMsg().get()->GetPackedBytes(&m_MsgBuffer[0], MAX_MSG_BUFFER_SIZE_IN_BYTES);
        DEBUG_TRACE("Sending wire message Message(MsgId:" << msgPtr.get()->GetId() << "), " << "buffer_length:" << buffer.second);

        PacketLength packetLength(buffer.second);
        if ((errorCode = SendData(packetLength.Data(), packetLength.Size())) == STATUS_SUCCESS)
        {
            errorCode = SendData(buffer.first, buffer.second);
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
        /// @@@ TODO:
        /// We are in exception state. Just drop the message???
    }
}

void TCPIOReceiver::Run()
{
    RELEASE_TRACE("TCPIOReceiver::Run");
    while (true)
    {
        // transfer the length first
        PacketLength packetLength;
        int numOfBytesReceived = recv(m_socket, packetLength.Data(), packetLength.Size(), 0);
        if (numOfBytesReceived < 1)
        {
            HandleException();
            break;
        }

        // We received the message length. Now, transfer the actual message.
        DEBUG_TRACE("Successfully received the data length: " << numOfBytesReceived);
        numOfBytesReceived = recv(m_socket, m_MsgBuffer, packetLength.Get(), 0);
        if (numOfBytesReceived < 1)
        {
            HandleException();
            break;
        }

        // Construct the message and send it to the upper layer.
        DEBUG_TRACE("Successfully received all the data: " << numOfBytesReceived);
        WireMsgPtr wireMsgPtr = WireMsgFactory::ConstructMsg(&m_MsgBuffer[0], numOfBytesReceived);
        m_p_connection->ProcessReceivedMsg(wireMsgPtr);
    }
}

void TCPIOReceiver::HandleException()
{
    DEBUG_TRACE("TCPIOReceiver::HandleException");
    /// @@@ TODO: close socket.
    // close(m_socket);
    // Clean up the connections.
}