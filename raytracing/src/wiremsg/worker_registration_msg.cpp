#include "worker_registration_msg.hpp"

/////////////////////////////////////////////////////////////////////////
/// Custom Message serializer
///
/// @param [ostrm] Output stream where the message is being serialized.
///
/////////////////////////////////////////////////////////////////////////
void WorkerRegistrationMsg::Pack(std::ostream& ostrm)
{
   WireMsg::Pack(ostrm);
   ostrm << m_hostname << " ";
   ostrm << m_Port << " ";
   ostrm << m_hw_thread_concurrency_level << " ";
   ostrm << m_PixelProductionTimeInSecForKnownScene << " ";
}

/////////////////////////////////////////////////////////////////////////
///  Custom message deserializer
///
///  @param [istrm] Input stream from which the message is being deserialized.
///
/////////////////////////////////////////////////////////////////////////
void WorkerRegistrationMsg::Unpack(std::istream& istrm)
{
   WireMsg::Unpack(istrm);
   istrm >> m_hostname >> m_Port >> m_hw_thread_concurrency_level >> m_PixelProductionTimeInSecForKnownScene;
}


/////////////////////////////////////////////////////////////////////////
/// Custom Message serializer
///
/// @param [ostrm] Output stream where the message is being serialized.
///
/////////////////////////////////////////////////////////////////////////
void WorkerRegistrationRespMsg::Pack(std::ostream& ostrm)
{
   WireMsg::Pack(ostrm);
   ostrm << m_ErrorCode << " ";
}

/////////////////////////////////////////////////////////////////////////
///  Custom message deserializer
///
///  @param [istrm] Input stream from which the message is being deserialized.
///
/////////////////////////////////////////////////////////////////////////
void WorkerRegistrationRespMsg::Unpack(std::istream& istrm)
{
   WireMsg::Unpack(istrm);
   istrm >> m_ErrorCode;
}


