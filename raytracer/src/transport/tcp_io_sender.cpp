#include "tcp_io_sender.hpp"

void TCPIOSender::Run()
{
   std::cerr << "Started TCPIOSender::Run thread" << std::endl;
   while (1)
   {
      std::cerr << "Running TCPIOSender::Run thread" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
   }
}
