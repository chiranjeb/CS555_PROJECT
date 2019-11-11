#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <queue>
#include <map>
#include "defines/defines_includes.hpp"
#include "framework/framework_includes.hpp"
#include "wiremsg/wire_msg.hpp"

class TCPIOSender;
class TCPIOReceiver;

class TCPIOConnection
{
public:
    static const int TCP_SEND_Q_DEPTH = 16;

public:
    /// Constructor
    TCPIOConnection(int socket, std::string clientIpAddress);

    /// Return the remote address.
    std::string GetRemoteAddress()
    {
        return m_ip;
    }

    /// Send message
    void SendMsg(WireMsgPtr wireMsg, Listener *p_lis);

    /// Process received message
    void ProcessReceivedMsg(WireMsgPtr wireMsg);

    /// Start the receiver
    void Start();

    /// Returns the socket asscociated with this connection.
    int GetSocket()
    {
        return m_Socket;
    }
private:
    /// remote ip connected to this machine.
    std::string m_ip;

    /// Asscociated socket with this TCP connection
    int m_Socket;

    /// Associated sender thread with this TCP connection
    TCPIOSender  *m_pIOSender;

    /// Associated receiver thread with this TCP connection
    TCPIOReceiver  *m_pIOReceiver;

    //bool m_WeInitiatedClose;

    std::queue<int> m_AppTagQ;
    std::map<int, Listener *> m_ClientRespRoutingMap;
    BlockingQueue<MsgPtr> m_SendQ;
};
