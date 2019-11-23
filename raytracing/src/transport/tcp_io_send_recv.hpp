#pragma once
#include<iostream>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOConnection;

// Consider changing this.
static const int MAX_MSG_BUFFER_SIZE_IN_BYTES = 4096;

struct PacketLength
{

    static const int MAX_PACKET_SIZE_LENGTH = 4;
    PacketLength()
    {
    }
    PacketLength(int value)
    {
        m_MsgLengthBuff[0] = (value & 0x000000FF);
        m_MsgLengthBuff[1] = (value & 0x0000FF00) >> 8;
        m_MsgLengthBuff[2] = (value & 0x00FF0000) >> 16;
        m_MsgLengthBuff[3] = (value & 0xFF000000) >> 24;
    }

    uint8_t* Data()
    {
        return &m_MsgLengthBuff[0];
    }

    int Size()
    {
        return sizeof(m_MsgLengthBuff);
    }

    int Get()
    {
        return m_MsgLengthBuff[3] << 24 | m_MsgLengthBuff[2] << 16 | m_MsgLengthBuff[1] << 8 | m_MsgLengthBuff[0];
    }

    uint8_t m_MsgLengthBuff[MAX_PACKET_SIZE_LENGTH];
};

class TCPIOSender : public Thread
{
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

    /// Send data
    ErrorCode_t SendData(uint8_t *data, int size);

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
    uint8_t m_MsgBuffer[MAX_MSG_BUFFER_SIZE_IN_BYTES];
};

class TCPIOReceiver  : public Thread
{
public:
    TCPIOReceiver(TCPIOConnection *p_connection, int socket) :
        m_p_connection(p_connection), m_socket(socket)
    {
    }

    void Start()
    {
        m_thread = new std::thread(&TCPIOReceiver::Run, *this);
    }

protected:
    void Run();
    void HandleException();
    uint8_t m_MsgBuffer[MAX_MSG_BUFFER_SIZE_IN_BYTES];

    int m_socket;
    TCPIOConnection *m_p_connection;
};
