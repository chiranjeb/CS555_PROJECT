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
#include "client.h"
#include "server_observer.h"
#include "pipe_ret_t.h"
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

#define MAX_PACKET_SIZE 4096

class TcpServer
{
private:

   int m_sockfd;
   struct sockaddr_in m_serverAddress;
   struct sockaddr_in m_clientAddress;
   fd_set m_fds;
   std::vector<Client> m_clients;
   std::vector<server_observer_t> m_subscibers;
   std::thread *threadHandle;

   void publishClientMsg(const Client& client, const char *msg, size_t msgSize);
   void publishClientDisconnected(const Client& client);
   void receiveTask(/*void * context*/);


public:

   pipe_ret_t start(int port);
   Client acceptClient(uint timeout);
   bool deleteClient(Client& client);
   void subscribe(const server_observer_t& observer);
   void unsubscribeAll();
   pipe_ret_t sendToAllClients(const char *msg, size_t size);
   pipe_ret_t sendToClient(const Client& client, const char *msg, size_t size);
   pipe_ret_t finish();
   void printClients();
};




class TCPIOServer : public Thread
{
public:
   void Construct(int listeningPort, int listeningDepth, Listener &serverResponseHandler);

   void Start()
   {
      m_thread = new std::thread(&TCPIOServer::Run, *this);
   }

   void Run();

   int m_listeningPort;
   int m_listeningQDepth;
   Listener *m_pserverResponseHandler;
   int m_sockfd;
   struct sockaddr_in m_serverAddress;
   struct sockaddr_in m_clientAddress;
   fd_set m_fds;
   std::vector<Client> m_clients;
   std::vector<server_observer_t> m_subscibers;
};



