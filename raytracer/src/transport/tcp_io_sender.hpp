#pragma once
#include <iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOConnection;
class TCPIOSender  : public Thread
{
    static const int MAX_SEND_Q_SIZE = 10;
public:
    TCPIOSender(TCPIOConnection *p_connection, int socket, BlockingQueue<MsgPtr> &sendQ) :
        m_socket(socket), m_p_connection(p_connection),  m_SendQ(sendQ)
    {
    }

    void Start()
    {
        m_thread = new std::thread(&TCPIOSender::Run, *this);
    }

    void SendMsg(MsgPtr msg)
    {
        m_SendQ.Put(msg);
    }



protected:
    void OnTCPSendMsg(MsgPtr requestMsgPtr);

    enum State
    {
        STATE_RUNNING,
        STATE_EXCEPTION
    };
    void Run();

    int m_socket;
    TCPIOConnection *m_p_connection;
    BlockingQueue<MsgPtr> &m_SendQ;
    bool m_Stop;
    State m_State;

    static const int MAX_PACKET_SIZE_LENGTH = 4;
    static const int MAX_MSG_BUFFER_SIZE_IN_BYTES = 4196;
    char m_MsgLengthBuff[MAX_PACKET_SIZE_LENGTH];
    char m_MsgBuffer[MAX_MSG_BUFFER_SIZE_IN_BYTES];

};
