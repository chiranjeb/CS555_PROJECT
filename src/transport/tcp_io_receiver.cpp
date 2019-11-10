#include "tcp_io_receiver.hpp"
#include <sys/types.h>
#include <sys/socket.h>

void TCPIOReceiver::Run()
{
   std::cerr << "TCPIOReceiver::Run" << std::endl;
   while (true)
   {
      int numOfBytesReceived = recv(m_socket, m_MsgLengthBuff, MAX_PACKET_SIZE_LENGTH, 0);
      if (numOfBytesReceived < 1)
      {
         HandleException();
         break;
      }

      // We received the message length. Now, Compute the message size and transfer the actual message.
      int messageLength = m_MsgLengthBuff[3] << 24 | m_MsgLengthBuff[2] << 16 | m_MsgLengthBuff[1] << 8 | m_MsgLengthBuff[0];
      numOfBytesReceived = recv(m_socket, m_MsgBuffer, messageLength, 0);
      if (numOfBytesReceived < 1)
      {
         HandleException();
         break;
      }

      // We have a message now. Need to construct it and send it upper layer.
      //publishClientMsg(*client, msg, numOfBytesReceived);
   }
}

void TCPIOReceiver::HandleException()
{
   /*
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
