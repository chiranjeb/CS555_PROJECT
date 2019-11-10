#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOReceiver  : public Thread
{
public:
   TCPIOReceiver(int socket):m_socket(socket)
   {
   }

   void Start()
   {
      m_thread = new std::thread(&TCPIOReceiver::Run, *this);
   }

protected:
   void Run();
   void HandleException();

   static const int MAX_PACKET_SIZE_LENGTH = 4;
   static const int MAX_MSG_BUFFER_SIZE_IN_BYTES = 4196;
   char m_MsgLengthBuff[MAX_PACKET_SIZE_LENGTH];
   char m_MsgBuffer[MAX_MSG_BUFFER_SIZE_IN_BYTES];

   int m_socket;
};