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

   uint16_t m_startY;
   uint16_t m_startX;
   uint16_t m_endY;
   uint16_t m_endX;
   uint32_t m_NumPixels;
   uint32_t m_ScenePixelOffset;
};


class PixelProduceRequestMsg : public WireMsg
{
public:
   /// PixelProduceRequestMsg message constructor
   PixelProduceRequestMsg(std::size_t scene_id, int numWorkLoad);

   /// PixelProduceRequestMsg message constructor
   PixelProduceRequestMsg() : WireMsg(MsgIdPixelProduceRequest)
   {
   }

   /// Return number of request
   uint16_t GetNumRequests() { return m_num_request; }

   /// Generate Work order.
   void GenerateWork(int index, uint16_t startY, uint16_t startX, uint16_t endY, uint16_t endX)
   {
      m_pixel_produce_request[index].GenerateWork(startY, startX, endY, endX);
   }

   /// Set pixel domain
   void SetPixelDomain(int index, uint32_t offset, uint32_t numPixels)
   {
      m_pixel_produce_request[index].SetPixelDomain(offset, numPixels);
   }

   /// Return request
   PixelProduceRequest* GetRequest(int index) { return &m_pixel_produce_request[index];}

   /// Custom Message serializer
   void Pack(std::ostream& ostrm);

   /// Custom message deserializer
   void Unpack(std::istream& istrm);

   /// PixelProduceRequestMsg message destructor
   ~PixelProduceRequestMsg();

   std::size_t GetSceneId() { return m_scene_id;}

   uint32_t GetNumPixels(int index) { return m_pixel_produce_request[index].m_NumPixels;}
   uint32_t GetScenePixelOffset(int index) { return m_pixel_produce_request[index].m_ScenePixelOffset; }

protected:
   std::size_t m_scene_id;
   PixelProduceRequest *m_pixel_produce_request;
   uint32_t m_num_request;
   uint32_t m_NX;
   uint32_t m_NY;
};



class PixelProduceResponseMsg : public WireMsg
{
public:
   /// PixelProduceRequestMsg message constructor
   PixelProduceResponseMsg(std::size_t scene_id);

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
      DEBUG_TRACE("PixelProduceResponseMsg: Destructor");
   }

protected:
   std::size_t m_scene_id;
   uint8_t *m_PackedMsgBuffer;
   uint8_t m_PackedMsgBufferLength;
};


typedef std::shared_ptr<PixelProduceRequestMsg>  PixelProduceRequestMsgPtr;
typedef std::shared_ptr<PixelProduceResponseMsg> PixelProduceResponseMsgPtr;
