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


        DEBUG_TRACE("!!!!!!!!!!!!!!!!Sender Exiting!!!!!!!!!!!!!!!");
        ///@@@ Need to handle connection fault
    }
}

ErrorCode_t TCPIOSender::SendData(uint8_t *data, int size)
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
        std::pair<uint8_t *, int> buffer = tcpSendMsg->GetWireMsg().get()->GetPackedBytes(&m_MsgBuffer[0], MAX_MSG_BUFFER_SIZE_IN_BYTES);
        DEBUG_TRACE("Sending wire message Message(MsgId:" << tcpSendMsg->GetWireMsg().get()->GetId() << "), " << "buffer_length:" << buffer.second);

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

        uint8_t *xfer_buffer = (uint8_t *)malloc(sizeof(uint8_t) * packetLength.Get());

        numOfBytesReceived = 0;
        DEBUG_TRACE("Successfully received the data length: " << packetLength.Get());
        while (packetLength.Get() !=numOfBytesReceived)
        {
           // We received the message length. Now, transfer the actual message.
           int numBytes = recv(m_socket, xfer_buffer+numOfBytesReceived, packetLength.Get() - numOfBytesReceived, 0)
           numOfBytesReceived += numBytes;
           if (numBytes < 1)
           {
               HandleException();
               break;
           }
           DEBUG_TRACE("Successfully received data: " << numOfBytesReceived);
           // Construct the message and send it to the upper layer.
        }
        DEBUG_TRACE("Successfully received all the data: " << numOfBytesReceived);
        WireMsgPtr wireMsgPtr = WireMsgFactory::ConstructMsg(xfer_buffer, numOfBytesReceived);
        wireMsgPtr->SetBufferContainer(xfer_buffer, packetLength.Get());
        m_p_connection->ProcessReceivedMsg(wireMsgPtr);
    }

}

void TCPIOReceiver::HandleException()
{
    DEBUG_TRACE("!!!!!!!!!!!!!!!!!!!!!!TCPIOReceiver::HandleException!!!!!!!!!!!!!!!!");
    /// @@@ TODO: close socket.
    // close(m_socket);
    // Clean up the connections.
}
