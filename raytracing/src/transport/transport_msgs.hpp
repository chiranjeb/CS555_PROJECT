#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/wire_msg.hpp"
#include "tcp_io_connection.hpp"

class TCPIOConnection;

class TCPServerConstructStatusMsg : public StatusMsg
{
public:
    TCPServerConstructStatusMsg(ErrorCode_t errorCode, int serverPort)
        : StatusMsg(MsgIdServerConstructResponse, errorCode), m_port(serverPort)
    { }

    int GetPort()
    {
        return m_port;
    }

protected:
    int m_port;
};

class TCPSendMsg : public Msg
{
public:
    TCPSendMsg(WireMsgPtr wireMsgPtr, ListenerPtr lis)
        : Msg(MsgIdTCPSend), m_WireMsg(wireMsgPtr), m_p_lis(lis)
    { }

    WireMsgPtr GetWireMsg()
    {
        return m_WireMsg;
    }

    ListenerPtr GetLis()
    {
        return m_p_lis;
    }

    ~TCPSendMsg()
    {
        m_WireMsg = nullptr;
        m_p_lis = nullptr;
    }

protected:
    WireMsgPtr m_WireMsg;
    ListenerPtr m_p_lis;
};

class TCPRecvMsg : public Msg
{
public:
    TCPRecvMsg(TCPIOConnectionPtr p_connection, WireMsgPtr wireMsg)
        : Msg(MsgIdTCPRecv), m_p_connection(p_connection), m_WireMsg(wireMsg)
    { }

    TCPIOConnectionPtr GetConnection()
    {
        return m_p_connection;
    }

    WireMsgPtr GetWireMsg()
    {
        return m_WireMsg;
    }

    ~TCPRecvMsg()
    {
        m_p_connection = nullptr;
    }

    WireMsgPtr m_WireMsg;
    TCPIOConnectionPtr m_p_connection;
};


class TCPSendStatusMsg : public Msg
{
public:
    TCPSendStatusMsg(int wireMsgId, ErrorCode_t errorCode)
        : Msg(MsgIdXmitStatus), m_WireMsgIdSent(wireMsgId), m_errorCode(errorCode)
    { }

    int GetSentWireMsgId()
    {
        return m_WireMsgIdSent;
    }

    int GetErrorCode()
    {
        return m_errorCode;
    }

    int GetWireMessageSent()
    {
        return m_WireMsgIdSent;
    }

    int m_WireMsgIdSent;
    ErrorCode_t m_errorCode;
};


class TCPConnectionEstablishRespMsg : public StatusMsg
{
public:
    /** 
    * Wire message constructor
    *  
    */
    TCPConnectionEstablishRespMsg(ErrorCode_t errorCode, TCPIOConnectionPtr p_connection, std::string& serverAddress, int listeningPort)
        : StatusMsg(MsgIdConnectionEstablishmentResponse, errorCode), m_ServerAddress(serverAddress)
    {
        m_p_connection = p_connection;
        m_ServerAddress = serverAddress;
        m_ListeningPort = listeningPort;
    }

    TCPIOConnectionPtr GetConnection()
    {
        return m_p_connection;
    }

    int GetServerPort()
    {
        return m_ListeningPort;
    }

    std::string& GetServerAddress()
    {
        return m_ServerAddress;
    }

    ~TCPConnectionEstablishRespMsg()
    {
        m_p_connection = nullptr;
    }

    TCPIOConnectionPtr m_p_connection;
    std::string& m_ServerAddress;
    int m_ListeningPort;
};



class TCPConnectionExceptionMsg : public Msg
{
public:
    /// Wire message constructor
    TCPConnectionExceptionMsg(TCPIOConnectionPtr p_connection)
        : Msg(MsgIdTCPConnectionException), m_p_connection(p_connection)
    {
    }

    TCPIOConnectionPtr GetConnection()
    {
        return m_p_connection;
    }

    ~TCPConnectionExceptionMsg()
    {
        m_p_connection = nullptr;
    }

    TCPIOConnectionPtr m_p_connection;
};

typedef std::shared_ptr<TCPSendMsg> TCPSendMsgPtr;
typedef std::shared_ptr<TCPRecvMsg> TCPRecvMsgPtr;
typedef std::shared_ptr<TCPSendStatusMsg> TCPSendStatusMsgPtr;
typedef std::shared_ptr<TCPConnectionEstablishRespMsg> TCPConnectionEstablishRespMsgPtr;
typedef std::shared_ptr<TCPConnectionExceptionMsg> TCPConnectionExceptionMsgPtr;

