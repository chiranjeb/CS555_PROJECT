#include <sys/types.h>
#include <sys/socket.h>
#include "tcp_io_receiver.hpp"
#include "transport_msgs.hpp"
#include "tcp_io_connection.hpp"
#include "wiremsg/wire_msg_factory.hpp"

void TCPIOReceiver::Run()
{
    RELEASE_TRACE("TCPIOReceiver::Run");
    while (true)
    {
        int numOfBytesReceived = recv(m_socket, m_MsgLengthBuff, MAX_PACKET_SIZE_LENGTH, 0);

        DEBUG_TRACE("TCPIOReceiver::rev something");
        if (numOfBytesReceived < 1)
        {
            HandleException();
            break;
        }


        DEBUG_TRACE("Successfully received the data length" << numOfBytesReceived);

        // We received the message length. Now, Compute the message size and transfer the actual message.
        int messageLength = m_MsgLengthBuff[3] << 24 | m_MsgLengthBuff[2] << 16 | m_MsgLengthBuff[1] << 8 | m_MsgLengthBuff[0];
        numOfBytesReceived = recv(m_socket, m_MsgBuffer, messageLength, 0);
        if (numOfBytesReceived < 1)
        {
            HandleException();
            break;
        }


        DEBUG_TRACE("Successfully receivedall the data" << numOfBytesReceived);

        // Construct the message and send it to the upper layer.
        WireMsgPtr wireMsgPtr = WireMsgFactory::ConstructMsg(&m_MsgBuffer[0], numOfBytesReceived);
        m_p_connection->ProcessReceivedMsg(wireMsgPtr);
    }
}

void TCPIOReceiver::HandleException()
{
    //close(m_socket);
    // Clean up the connections.

    /* 
    
    close(client->getFileDescriptor());
    client->setDisconnected();
    if (numOfBytesReceived == 0) //client closed connection
    {
       client->setErrorMessage("Client closed connection");
       //printf("client closed");
    } else
    {
       client->setErrorMessage(strerror(errno));
    }
    close(client->getFileDescriptor());
    publishClientDisconnected(*client);
    deleteClient(*client);
    */
}
