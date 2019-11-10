#pragma once
#include "defines/error_codes.hpp"
#include<memory>

class Msg
{
public:
   Msg(int id = 0)
   {
      m_id = id;
   }
   int GetId()
   {
      return m_id;
   }
   int m_id;
};

typedef std::shared_ptr<Msg> MsgPtr;

class StatusMsg : public Msg
{
public:
   StatusMsg(int msgId, ErrorCode_t errorCode) : Msg(msgId), m_errorCode(errorCode)
   {
   }
   ErrorCode_t m_errorCode;
};

typedef std::shared_ptr<StatusMsg> StatusMsgPtr;



