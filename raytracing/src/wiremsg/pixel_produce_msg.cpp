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
   ostrm << m_ThreadId << " ";
   ostrm << m_AppTag << " ";
}

void PixelProduceRequest::Unpack(std::istream& istrm)
{
   istrm >> m_startY >> m_startX >> m_endY 
         >> m_endX >> m_NumPixels 
         >> m_ScenePixelOffset >> m_ThreadId >> m_AppTag;
}

/// PixelProduceRequestMsg message constructor
PixelProduceRequestMsg::PixelProduceRequestMsg(std::size_t sceneId, int numWorkLoad) : WireMsg(MsgIdPixelProduceRequest)
{
   m_SceneId = sceneId;
   m_NumRequest = numWorkLoad;
   m_pPixelProduceRequest = (PixelProduceRequest *)malloc(sizeof(PixelProduceRequest) * m_NumRequest);
   for (uint16_t index = 0; index < m_NumRequest; ++index)
   {
       m_pPixelProduceRequest[index].m_ThreadId = index;
   }
   DEBUG_TRACE_WIRE_MSG("PixelProduceRequestMsg: Constructor");
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
   DEBUG_TRACE_WIRE_MSG("PixelProduceRequestMsg:Pack");
   WireMsg::Pack(ostrm);
   ostrm << m_SceneId << " ";
   ostrm << m_NumRequest << " ";
   for (int index = 0; index < m_NumRequest; index++)
   {
      m_pPixelProduceRequest[index].Pack(ostrm);
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
   istrm >> m_SceneId;
   istrm >> m_NumRequest;
   m_pPixelProduceRequest = (PixelProduceRequest *)malloc(sizeof(PixelProduceRequest) * m_NumRequest);
   for (int index = 0; index < m_NumRequest; index++)
   {
      m_pPixelProduceRequest[index].Unpack(istrm);
   }
}

PixelProduceRequestMsg::~PixelProduceRequestMsg()
{
   DEBUG_TRACE_WIRE_MSG("PixelProduceRequestMsg: Destructor");
   free(m_pPixelProduceRequest);
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
   DEBUG_TRACE_WIRE_MSG("PixelProduceResponseMsg:Pack");
   WireMsg::Pack(ostrm);
   ostrm << m_SceneId << " ";
   ostrm << m_NumPixels << " ";
   ostrm << m_ScenePixelOffset << " ";
   ostrm << m_ThreadId << " ";
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
   istrm >> m_SceneId >> m_NumPixels 
         >> m_ScenePixelOffset >> m_ThreadId ;
}


