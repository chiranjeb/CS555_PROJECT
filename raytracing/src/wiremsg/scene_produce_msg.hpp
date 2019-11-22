#pragma once
#include "wire_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"


class SceneProduceRequestMsg : public WireMsg
{
public:
   // SceneProduceRequestMsg message constructor
   SceneProduceRequestMsg(SceneDescriptorPtr sceneDescriptorPtr) :
      WireMsg(MsgIdSceneProduceRequest), m_SceneDescriptorPtr(sceneDescriptorPtr)
   {
      DEBUG_TRACE("MsgIdSceneProduceRequest: Constructor");
   }

   /// Scene distribution message.
   SceneProduceRequestMsg() : WireMsg(MsgIdSceneProduceRequest)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom Message serializer
   /// @param [ostrm] Output stream where the message is being
   ///            serialized to
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream& ostrm)
   {
      DEBUG_TRACE("SceneProduceRequestMsg:Pack");
      WireMsg::Pack(ostrm);
      ostrm << m_NX << " ";
      ostrm << m_NY << " ";
      ostrm << m_SceneId << " ";
      ostrm << m_IPAddress << " ";
      ostrm << m_Port << " ";
      ostrm << m_BufferLength << " ";
      m_SceneDescriptorPtr.get()->Pack(ostrm);
   }


   /// Update the answer back address.
   void SetAnswerBackAddress(std::string& hostname, int port)
   {
      m_IPAddress = hostname;
      m_Port = port;
   }

   /// Set image dimension
   void SetImageDimension(uint32_t NX, uint32_t NY)
   {
      m_NX = NX;
      m_NY = NY;
   }

   /// Set scene id
   void SetSceneId(std::size_t sceneId)
   {
      m_SceneId = sceneId;
   }

   std::size_t GetSceneId()
   {
      return m_SceneId;
   }

   /// Get NX and NY
   uint32_t GetNX() { return m_NX;}
   uint32_t GetNY() { return m_NY;}
   std::string& GetClientAddress() { return m_IPAddress; }
   int GetClientPort() { return m_Port; }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom message deserializer
   /// @param [istrm] Input stream from which the message is being
   ///           deserialized.
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream& istrm)
   {
      WireMsg::Unpack(istrm);
      istrm >> m_NX >> m_NY >> m_SceneId >> m_IPAddress;
      istrm >> m_Port >> m_BufferLength;

#if !defined(__Make_master)
      m_SceneDescriptorPtr = std::make_shared<SceneDescriptor>();
      m_SceneDescriptorPtr.get()->Unpack(istrm);
#endif
   }

   SceneDescriptorPtr GetSceneDescriptor()
   {
      return m_SceneDescriptorPtr;
   }

   ~SceneProduceRequestMsg()
   {
      DEBUG_TRACE("SceneProduceRequestMsg: Destructor");
      free(m_PackedMsgBuffer);
   }

   uint32_t m_NX;
   uint32_t m_NY;
   std::size_t m_SceneId;
   std::string m_IPAddress;
   uint32_t m_Port;
   uint32_t m_BufferLength;
   SceneDescriptorPtr  m_SceneDescriptorPtr;
};


class SceneSegmentProduceResponseMsg : public WireMsg
{
public:
   /// SceneSegmentProduceResponseMsg message constructor
   SceneSegmentProduceResponseMsg(std::size_t sceneId, uint32_t numPixels, uint32_t scenePixelOffset) :
      WireMsg(MsgIdSceneSegmentProduceResponse),
      m_SceneId(sceneId), m_NumPixels(numPixels),
      m_ScenePixelOffset(scenePixelOffset)
   {
      m_BufferLength = 6 * numPixels;
      uint32_t totalPackSizeinBytes = sizeof(*this) + 128 + m_BufferLength;
      SetBufferContainer(m_PackedMsgBuffer, m_PackedMsgBufferLength);
      m_PackedMsgBuffer = (uint8_t *)malloc(sizeof(uint8_t) * totalPackSizeinBytes);
      m_PackedMsgBufferLength = totalPackSizeinBytes;
      DEBUG_TRACE("SceneSegmentProduceResponseMsg: Constructor");
   }

   /// Scene segment produce response message.
   SceneSegmentProduceResponseMsg() : WireMsg(MsgIdSceneSegmentProduceResponse)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom Message serializer
   /// @param [ostrm] Output stream where the message is being
   ///            serialized to
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream& ostrm)
   {
      DEBUG_TRACE("SceneSegmentProduceResponseMsg:Pack");
      WireMsg::Pack(ostrm);
      ostrm << m_SceneId << " " << m_NumPixels << " " << m_ScenePixelOffset << " " << m_BufferLength;
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom message deserializer
   /// @param [istrm] Input stream from which the message is being
   ///           deserialized.
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream& istrm)
   {
      WireMsg::Unpack(istrm);
      istrm >> m_SceneId >> m_NumPixels >> m_ScenePixelOffset >> m_BufferLength;
   }

   ~SceneSegmentProduceResponseMsg()
   {
      DEBUG_TRACE("SceneSegmentProduceResponseMsg: Destructor");
      free(m_PackedMsgBuffer);
   }

   std::pair<uint8_t *, uint32_t> GetSceneBuffer()
   {
      return std::pair<uint8_t *, uint32_t>(GetPixelBufferStart(), m_BufferLength);
   }

   uint32_t GetScenePixelOffset() { return m_ScenePixelOffset; }
   uint32_t GetNumPixels() { return m_NumPixels;}
   uint8_t* GetPixelBufferStart() { return (m_PackedMsgBuffer + 128); }
   uint32_t GetPixelBufferMaxLimit() { return m_BufferLength;}

   bool operator<(const SceneSegmentProduceResponseMsg& otherMsg) const
   {
      return (m_ScenePixelOffset < otherMsg.m_ScenePixelOffset);
   }

   std::size_t m_SceneId;
   uint32_t m_NumPixels;
   uint32_t m_ScenePixelOffset;
   uint32_t m_BufferLength;
};



class SceneProduceRequestAckMsg : public WireMsg
{
public:
   /** 
   * SceneProduceRequestMsg message constructor
   *  
   */
   SceneProduceRequestAckMsg(int appTag, ErrorCode_t errorCode) :
      WireMsg(MsgIdSceneProduceRequestAck)
   {
      DEBUG_TRACE("MsgIdSceneProduceRequest: Constructor");
      SetAppTag(appTag);
      m_errorCode = errorCode;
   }

   /// Scene distribution message.
   SceneProduceRequestAckMsg() : WireMsg(MsgIdSceneProduceRequestAck)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom Message serializer
   /// @param [ostrm] Output stream where the message is being
   ///            serialized to
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream& ostrm)
   {
      DEBUG_TRACE("SceneProduceRequestAckMsg:Pack");
      WireMsg::Pack(ostrm);
      ostrm << int(m_errorCode) << " ";
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom message deserializer
   /// @param [istrm] Input stream from which the message is being
   ///           deserialized.
   ///
   ///////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream& istrm)
   {
      WireMsg::Unpack(istrm);
      int errCode;
      istrm >> errCode;
      m_errorCode = ErrorCode_t(errCode);
   }

   ErrorCode_t GetErrorCode()
   {
      return m_errorCode;
   }

   ~SceneProduceRequestAckMsg()
   {
      DEBUG_TRACE("SceneProduceRequestAckMsg: Destructor");
   }

   ErrorCode_t m_errorCode;
};






typedef std::shared_ptr<SceneProduceRequestMsg> SceneProduceRequestMsgPtr;
typedef std::shared_ptr<SceneSegmentProduceResponseMsg> SceneSegmentProduceResponseMsgPtr;
typedef std::shared_ptr<SceneProduceRequestAckMsg> SceneProduceRequestAckMsgPtr;

