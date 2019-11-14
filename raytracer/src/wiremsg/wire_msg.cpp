#include "wire_msg.hpp"

///////////////////////////////////////////////////////////////////////////////////////
///  Returns true if a response is expected in response to sending
///  this message otherwise false.
///////////////////////////////////////////////////////////////////////////////////////
bool WireMsg::ExpectingRecvRecvResponse()
{
   if (GetAppTag() != 0)
   {
      return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Return a serialized version of the message
/// @return byte array and size pair
///////////////////////////////////////////////////////////////////////////////////////
std::pair<char *, int> WireMsg::GetPackedBytes(char *pre_allocated_buffer, int size)
{
   PreAllocatedStreamBuffer streambuff(pre_allocated_buffer, size);
   std::ostream ostrm(&streambuff);
   Pack(ostrm);
   return std::pair<char *, int>(pre_allocated_buffer, streambuff.Tellp());
}

///////////////////////////////////////////////////////////////////////////////////////
/// Custom Message serializer
/// @param ostrm Output stream where the message is being
///            serialized to
////////////////////////////////////////////////////////////////////////////////////////
void WireMsg::Pack(std::ostream &ostrm)
{
   DEBUG_TRACE("WireMsg:Pack");
   Msg::Pack(ostrm);
   ostrm << m_ApplicationTag << " ";
}

///////////////////////////////////////////////////////////////////////////////////////
/// Custom message deserializer
/// @param istr Input stream from which the message is being
///           deserialized.
///////////////////////////////////////////////////////////////////////////////////////
void WireMsg::Unpack(std::istream &istrm)
{
   Msg::Unpack(istrm);
   istrm >> m_ApplicationTag;
}


