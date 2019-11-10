#pragma once
#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
class TCPIOSender  : public Thread
{
public:
   TCPIOSender(int socket) : m_socket(socket)
   {
   }

   void Start()
   {
      m_thread = new std::thread(&TCPIOSender::Run, *this);
   }

protected:
   void Run();

   int m_socket;
};
