#pragma once
#include "defines/error_codes.hpp"
#include<memory>
#include<ostream>
#include<istream>

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


protected:
   ///  Unpack will not do anything. It's expected that the message
   /// type has been unpacked or determined first before
   /// constructing a message. Thus when the Unpack gets called,
   /// we don't need to populate the message Id. It will already be
   /// set up thorugh constructor invocation. Thus, This dummy
   /// function keeps symmetricity to the Pack with a no operation.
   ///
   /// @param istrm Input stream from the message is being recreated.
   ///
   virtual void Unpack(std::ostream istrm)
   {
      // This has been unpacked very first to identify the message id
   }

   /// Pack the message to a output stream.
   /// @param ostrm Output stream where the message is being packed
   ///            to.
   ///
   void Pack(DataOutputStream ostrm)  throws IOException
   {
      out << m_Id;
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



