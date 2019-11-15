#pragma once

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>
#include <cstring>
#include <errno.h>
#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

#define MAX_PACKET_SIZE 4096

class TCPIOConnection;
class TCPIOServer : public Thread
{
public:
   ErrorCode_t Construct(int listeningPort, int listeningDepth);

   TCPIOConnection* AcceptConnection();

   void Start()
   {
      m_thread = new std::thread(&TCPIOServer::Run, *this);
   }

   int GetPort()
   {
       return m_listeningPort;
   }

   void Run();

   int m_listeningPort;
   int m_listeningQDepth;
   int m_sockfd;
   struct sockaddr_in m_serverAddress;
   struct sockaddr_in m_clientAddress;
   fd_set m_fds;
};



