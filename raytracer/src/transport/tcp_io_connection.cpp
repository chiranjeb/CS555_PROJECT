#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <vector>
#include <errno.h>
#include <thread>
#include "tcp_io_connection.hpp"
#include "tcp_io_receiver.hpp"

TCPIOConnection::TCPIOConnection(int socket, std::string clientIpAddress)
{
   m_Socket = socket;
   m_ip = clientIpAddress;
   //m_SendQ = nullptr;
}

/** 
* Establish a connection initiated by client. 
* @throw IOException on IO error.
* @param socket client socket 
*/
void TCPIOConnection::Start()
{
   //m_SendQ = nullptr;
   m_pIOSender = nullptr;

   //If a TCP connection is used using socket. We will create a recvQ only and let
   //upper layer chose whether to create a send thread or not.
   //m_pIOReceiver = new TCPIOReceiver(m_Socket, this);
   m_pIOReceiver = new TCPIOReceiver(m_Socket);
   m_pIOReceiver->Start();
}
