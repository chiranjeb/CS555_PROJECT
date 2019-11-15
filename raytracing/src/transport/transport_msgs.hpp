#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/wire_msg.hpp"

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
   TCPSendMsg(WireMsgPtr wireMsgPtr, Listener *lis)
      : Msg(MsgIdTCPSend), m_WireMsg(wireMsgPtr), m_p_lis(lis)
   { }

   WireMsgPtr GetWireMsg()
   {
      return m_WireMsg;
   }

   Listener* GetLis()
   {
      return m_p_lis;
   }

protected:
   WireMsgPtr m_WireMsg;
   Listener  *m_p_lis;
};

class TCPRecvMsg : public Msg
{
public:
   TCPRecvMsg(TCPIOConnection *p_connection, WireMsgPtr wireMsg)
      : Msg(MsgIdTCPRecv), m_p_connection(p_connection), m_WireMsg(wireMsg)
   { }

   TCPIOConnection* GetConnection()
   {
      return m_p_connection;
   }

   WireMsgPtr GetWireMsg()
   {
      return m_WireMsg;
   }

   WireMsgPtr m_WireMsg;
   TCPIOConnection *m_p_connection;
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
   TCPConnectionEstablishRespMsg(ErrorCode_t errorCode, TCPIOConnection *p_connection, std::string& serverAddress, int listeningPort)
      : StatusMsg(MsgIdConnectionEstablishmentResponse, errorCode), m_ServerAddress(serverAddress)
   {
      m_p_connection = p_connection;
      m_ServerAddress = serverAddress;
      m_ListeningPort = listeningPort;
   }

   TCPIOConnection* GetConnection()
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


   TCPIOConnection *m_p_connection;
   std::string& m_ServerAddress;
   int m_ListeningPort;
};


