#include "wire_msg.hpp"

///////////////////////////////////////////////////////////////////////////////////////
///  Returns true if a response is expected from remote in response to
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
std::pair<uint8_t *, int> WireMsg::GetPackedBytes(uint8_t *pre_allocated_buffer, int size)
{
   if (m_PackedMsgBuffer == nullptr)
   {
      PreAllocatedStreamBuffer streambuff(reinterpret_cast<char*>(pre_allocated_buffer), size);
      std::ostream ostrm(&streambuff);
      Pack(ostrm);
      return std::pair<uint8_t *, int>(pre_allocated_buffer, streambuff.Tellp());
   }
   else
   {
      return std::pair<uint8_t *, int>(m_PackedMsgBuffer, m_PackedMsgBufferLength);
   }
}

///////////////////////////////////////////////////////////////////////////////////////
/// Custom Message serializer
/// @param [ostrm] Output stream where the message is being serialized to
////////////////////////////////////////////////////////////////////////////////////////
void WireMsg::Pack(std::ostream& ostrm)
{
   Msg::Pack(ostrm);
   ostrm << m_ApplicationTag << " ";
}

///////////////////////////////////////////////////////////////////////////////////////
/// Custom message deserializer
/// @param [istr] Input stream from which the message is being deserialized.
///////////////////////////////////////////////////////////////////////////////////////
void WireMsg::Unpack(std::istream& istrm)
{
   Msg::Unpack(istrm);
   istrm >> m_ApplicationTag;
}


void WireMsg::Repack()
{
   PreAllocatedStreamBuffer streambuff(reinterpret_cast<char*>(m_PackedMsgBuffer), m_PackedMsgBufferLength);
   std::ostream ostrm(&streambuff);

   // Just pack the id and application tag.
   Pack(ostrm);
}


