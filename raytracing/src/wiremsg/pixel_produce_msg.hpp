#pragma once
#include "wire_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"

class PixelProduceRequestMsg : public WireMsg
{
public:
    /// PixelProduceRequestMsg message constructor
    PixelProduceRequestMsg(std::size_t scene_id) : WireMsg(MsgIdPixelProduceRequest)
    {
        m_scene_id = scene_id;
        DEBUG_TRACE("PixelProduceRequestMsg: Constructor");
    }

    /// Scene distribution message.
    PixelProduceRequestMsg() : WireMsg(MsgIdPixelProduceRequest)
    {
    }

    /// Generate Work order.
    void GenerateWork(int startY, int startX, int endY, int endX)
    {
        m_startY = startY;
        m_startX = startX;
        m_endY = endY;
        m_endX = endX;
    }

    void SetPixelDomain(uint32_t offset, uint32_t numPixels)
    {
        m_ScenePixelOffset = offset;
        m_NumPixels = numPixels;
    }

    ///////////////////////////////////////////////////////////////////////////
    ///
    /// Custom Message serializer
    /// @param [ostrm] Output stream where the message is being
    ///            serialized to
    ///
    ///////////////////////////////////////////////////////////////////////////
    void Pack(std::ostream &ostrm)
    {
        DEBUG_TRACE("PixelProduceRequestMsg:Pack");
        WireMsg::Pack(ostrm);

        ostrm << m_startY << " ";
        ostrm << m_startX << " ";
        ostrm << m_endY << " ";
        ostrm << m_endX << " ";
        ostrm << m_scene_id << " ";
        ostrm << m_NumPixels << " ";
        ostrm << m_ScenePixelOffset << " ";
    }

    ///////////////////////////////////////////////////////////////////////////
    ///
    /// Custom message deserializer
    /// @param [istrm] Input stream from which the message is being
    ///           deserialized.
    ///
    ///////////////////////////////////////////////////////////////////////////
    void Unpack(std::istream &istrm)
    {
        WireMsg::Unpack(istrm);
        istrm >> m_startY >> m_startX >> m_endY >> m_endX >> m_scene_id >> m_NumPixels >> m_ScenePixelOffset;
    }

    ~PixelProduceRequestMsg()
    {
        DEBUG_TRACE("PixelProduceRequestMsg: Destructor");
    }

    std::size_t GetSceneId() { return m_scene_id;}
    uint32_t GetNumPixels() { return m_NumPixels;}
    uint32_t GetScenePixelOffset() { return m_ScenePixelOffset; }

    std::size_t m_scene_id;
    uint32_t m_NX;
    uint32_t m_NY;
    uint32_t m_startY;
    uint32_t m_startX;
    uint32_t m_endY;
    uint32_t m_endX;
    uint32_t m_NumPixels;
    uint32_t m_ScenePixelOffset;
};


class PixelProduceResponseMsg : public WireMsg
{
public:
    /// PixelProduceRequestMsg message constructor
    PixelProduceResponseMsg(std::size_t scene_id) :
        WireMsg(MsgIdPixelProduceResponse)
    {
        m_scene_id = scene_id;
        DEBUG_TRACE("PixelProduceResponseMsg: Constructor");
    }

    /// Scene distribution message.
    PixelProduceResponseMsg() : WireMsg(MsgIdPixelProduceResponse)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    ///
    /// Custom Message serializer
    /// @param [ostrm] Output stream where the message is being
    ///            serialized to
    ///
    ///////////////////////////////////////////////////////////////////////////
    void Pack(std::ostream &ostrm)
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
    void Unpack(std::istream &istrm)
    {
        WireMsg::Unpack(istrm);
        istrm >> m_scene_id;
    }

    ~PixelProduceResponseMsg()
    {
        DEBUG_TRACE("PixelProduceResponseMsg: Destructor");
    }

    std::size_t m_scene_id;

    uint8_t *m_PackedMsgBuffer;
    uint8_t m_PackedMsgBufferLength;
};


typedef std::shared_ptr<PixelProduceRequestMsg>  PixelProduceRequestMsgPtr;
typedef std::shared_ptr<PixelProduceResponseMsg> PixelProduceResponseMsgPtr;
