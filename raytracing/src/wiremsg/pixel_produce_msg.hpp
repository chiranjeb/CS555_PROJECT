#pragma once
#include "wire_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"

struct PixelProduceRequest
{
   void GenerateWork(uint16_t startY, uint16_t startX, uint16_t endY, uint16_t endX);
   void SetPixelDomain(uint32_t offset, uint32_t numPixels);
   void Pack(std::ostream& ostrm);
   void Unpack(std::istream& istrm);

   uint32_t GetNumPixels() { return m_NumPixels;}
   uint32_t GetScenePixelOffset() { return m_ScenePixelOffset; }
   int GetAppTag() { return m_AppTag; }
   void SetupAppTag(int appTag) { m_AppTag = appTag;}
   void SetThreadId(uint16_t threadId) { m_ThreadId = threadId; }

   uint16_t m_ThreadId;
   uint16_t m_startY;
   uint16_t m_startX;
   uint16_t m_endY;
   uint16_t m_endX;
   uint32_t m_NumPixels;
   uint32_t m_ScenePixelOffset;
   int m_AppTag;
};


class PixelProduceRequestMsg : public WireMsg
{
public:
   /// PixelProduceRequestMsg message constructor
   PixelProduceRequestMsg(std::size_t sceneId, int numWorkLoad);

   /// PixelProduceRequestMsg message constructor
   PixelProduceRequestMsg() : WireMsg(MsgIdPixelProduceRequest)
   {
   }

   /// Return number of request
   uint16_t GetNumRequests() { return m_NumRequest; }

   PixelProduceRequest* Request(int index) { return &m_pPixelProduceRequest[index];}

   /// Generate Work order.
   void GenerateWork(int index, uint16_t startY, uint16_t startX, uint16_t endY, uint16_t endX)
   {
      m_pPixelProduceRequest[index].GenerateWork(startY, startX, endY, endX);
   }

   /// Set pixel domain
   void SetPixelDomain(int index, uint32_t offset, uint32_t numPixels)
   {
      m_pPixelProduceRequest[index].SetPixelDomain(offset, numPixels);
   }

   void SetupAppTag(int index, int appTag)
   {
       m_pPixelProduceRequest[index].SetupAppTag(appTag);
   }

   /// Return request
   PixelProduceRequest* GetRequest(int index) { return &m_pPixelProduceRequest[index];}

   /// Custom Message serializer
   void Pack(std::ostream& ostrm);

   /// Custom message deserializer
   void Unpack(std::istream& istrm);

   /// PixelProduceRequestMsg message destructor
   ~PixelProduceRequestMsg();

   std::size_t GetSceneId() { return m_SceneId;}

   uint32_t GetNumPixels(int index) { return m_pPixelProduceRequest[index].m_NumPixels;}
   uint32_t GetScenePixelOffset(int index) { return m_pPixelProduceRequest[index].m_ScenePixelOffset; }

protected:
   std::size_t m_SceneId;
   PixelProduceRequest *m_pPixelProduceRequest;
   uint32_t m_NumRequest;
   uint32_t m_NX;
   uint32_t m_NY;
   uint32_t m_NextRequestIndex;
};



class PixelProduceResponseMsg : public WireMsg
{
public:
   /// PixelProduceRequestMsg message constructor
   PixelProduceResponseMsg(std::size_t sceneId, uint32_t numPixels, uint32_t scenePixelOffset, uint16_t  threadId):
       WireMsg(MsgIdPixelProduceResponse), m_SceneId(sceneId), m_NumPixels(numPixels), m_ScenePixelOffset(scenePixelOffset), m_ThreadId(threadId)
   {
   }

   /// PixelProduceRequestMsg message constructor
   PixelProduceResponseMsg() : WireMsg(MsgIdPixelProduceResponse)
   {
   }

   /// Custom Message serializer
   void Pack(std::ostream& ostrm);

   /// Custom message deserializer
   void Unpack(std::istream& istrm);

   /// PixelProduceRequestMsg message destructor
   ~PixelProduceResponseMsg()
   {
      DEBUG_TRACE_WIRE_MSG("PixelProduceResponseMsg: Destructor");
   }

   void Dump()
   {
       DEBUG_TRACE_WIRE_MSG("PixelProduceResponseMsg::Dump - " << ", m_AppTag:" << m_ApplicationTag 
                   <<  ", m_SceneId:" << m_SceneId << ", m_ThreadId:" << m_ThreadId << ", m_ScenePixelOffset: " << m_ScenePixelOffset);
   }

   std::size_t GetSceneId() { return m_SceneId;}
   uint16_t GetThreadId()   { return m_ThreadId;}
   uint32_t GetNumPixels()  { return m_NumPixels;}
   uint32_t GetScenePixelOffset() { return m_ScenePixelOffset;}

protected:
   std::size_t m_SceneId;
   uint32_t m_NumPixels;
   uint32_t m_ScenePixelOffset;
   uint16_t m_ThreadId;
   PixelProduceRequest *m_pPixelProduceRequest;
};


typedef std::shared_ptr<PixelProduceRequestMsg>  PixelProduceRequestMsgPtr;
typedef std::shared_ptr<PixelProduceResponseMsg> PixelProduceResponseMsgPtr;
