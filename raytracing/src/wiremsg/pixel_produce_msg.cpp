#include "pixel_produce_msg.hpp"


void PixelProduceRequest::GenerateWork(uint16_t startY, uint16_t startX, uint16_t endY, uint16_t endX)
{
   m_startY = startY;
   m_startX = startX;
   m_endY = endY;
   m_endX = endX;
}

void PixelProduceRequest::SetPixelDomain(uint32_t offset, uint32_t numPixels)
{
   m_ScenePixelOffset = offset;
   m_NumPixels = numPixels;
}

void PixelProduceRequest::Pack(std::ostream& ostrm)
{
   ostrm << m_startY << " ";
   ostrm << m_startX << " ";
   ostrm << m_endY << " ";
   ostrm << m_endX << " ";
   ostrm << m_NumPixels << " ";
   ostrm << m_ScenePixelOffset << " ";
}

void PixelProduceRequest::Unpack(std::istream& istrm)
{
   istrm >> m_startY >> m_startX >> m_endY >> m_endX >> m_NumPixels >> m_ScenePixelOffset;
}

/// PixelProduceRequestMsg message constructor
PixelProduceRequestMsg::PixelProduceRequestMsg(std::size_t scene_id, int numWorkLoad) : WireMsg(MsgIdPixelProduceRequest)
{
   m_scene_id = scene_id;
   m_num_request = numWorkLoad;
   m_pixel_produce_request = (PixelProduceRequest *)malloc(sizeof(PixelProduceRequest) * m_num_request);
   DEBUG_TRACE("PixelProduceRequestMsg: Constructor");
}


///////////////////////////////////////////////////////////////////////////
///
/// Custom Message serializer
/// @param [ostrm] Output stream where the message is being
///            serialized to
///
///////////////////////////////////////////////////////////////////////////
void PixelProduceRequestMsg::Pack(std::ostream& ostrm)
{
   DEBUG_TRACE("PixelProduceRequestMsg:Pack");
   WireMsg::Pack(ostrm);
   ostrm << m_scene_id << " ";
   ostrm << m_num_request << " ";
   for (int index = 0; index < m_num_request; index++)
   {
      m_pixel_produce_request[index].Pack(ostrm);
   }
}

///////////////////////////////////////////////////////////////////////////
///
/// Custom message deserializer
/// @param [istrm] Input stream from which the message is being
///           deserialized.
///
///////////////////////////////////////////////////////////////////////////
void PixelProduceRequestMsg::Unpack(std::istream& istrm)
{
   WireMsg::Unpack(istrm);
   istrm >> m_scene_id;
   istrm >> m_num_request;
   m_pixel_produce_request = (PixelProduceRequest *)malloc(sizeof(PixelProduceRequest) * m_num_request);
   for (int index = 0; index < m_num_request; index++)
   {
      m_pixel_produce_request[index].Unpack(istrm);
   }
}

PixelProduceRequestMsg::~PixelProduceRequestMsg()
{
   DEBUG_TRACE("PixelProduceRequestMsg: Destructor");
   free(m_pixel_produce_request);
}

///////////////////////////////////////////////////////////////////////////
///
/// Custom Message serializer
/// @param [ostrm] Output stream where the message is being
///            serialized to
///
///////////////////////////////////////////////////////////////////////////
void PixelProduceResponseMsg::Pack(std::ostream& ostrm)
{
   DEBUG_TRACE("PixelProduceResponseMsg:Pack");
   WireMsg::Pack(ostrm);
   ostrm << m_scene_id << " ";
}

///////////////////////////////////////////////////////////////////////////
///
/// Custom message deserializer
/// @param [istrm] Input stream from which the message is being
///           deserialized.
///
///////////////////////////////////////////////////////////////////////////
void PixelProduceResponseMsg::Unpack(std::istream& istrm)
{
   WireMsg::Unpack(istrm);
   istrm >> m_scene_id;
}


