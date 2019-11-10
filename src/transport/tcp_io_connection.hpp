#pragma once

#include <iostream>
#include <memory>
#include <string>

class TCPIOSender;
class TCPIOReceiver;

class TCPIOConnection
{
public:
   std::string m_ip;
   static const int TCP_SEND_Q_DEPTH = 16;

public:
   /** 
   * Constructor 
   */
   TCPIOConnection(int socket, std::string clientIpAddress);
   

   std::string GetRemoteAddress()
   {
      return m_ip;
   }


   void Start();

   /** 
   * Returns the socket asscociated with this connection.
   * @return Socket
   *  
   */

   int GetSocket()
   {
      return m_Socket;
   }


private:
   /** 
   * Asscociated socket with this TCP connection
   */
   int m_Socket;

   /** 
   * Associated sender thread with this TCP connection
   */
   TCPIOSender  *m_pIOSender;

   /** 
   * Associated receiver thread with this TCP connection
   */
   TCPIOReceiver  *m_pIOReceiver;

   //bool m_WeInitiatedClose;

   /** 
   * SendQ asscociated with this TCP connection
   */
   //BlockingQueue<MsgQEntry> m_SendQ;
};
