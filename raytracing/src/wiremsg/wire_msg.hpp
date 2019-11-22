#pragma once
#include<memory>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

/**
 * Base class for all wire message.
 *  
 */
class TCPIOConnection;
class WireMsg : public Msg
{
public:
    /// Message constructor
    WireMsg(int msgId) : Msg(msgId)
    {
        m_ApplicationTag = 0;
        m_p_tcp_connection = nullptr;
        m_BufferValid = false;
    }

    ///  Returns the application tag
    int GetAppTag()
    {
        return m_ApplicationTag;
    }

    ///  Sets the application tag
    void SetAppTag(int contextId)
    {
        m_ApplicationTag = contextId;
    }

    ///  Returns true if a response is expected in response to sending
    ///  this message otherwise false.
    bool ExpectingRecvRecvResponse();

    ///  Return a serialized version of the message
    std::pair<uint8_t *, int> GetPackedBytes(uint8_t *pre_allocated_buffer, int size);

    ///  Custom Message serializer
    void Pack(std::ostream &ostrm);

    /// Repack
    void Repack();

    ///  Custom message deserializer
    void Unpack(std::istream &istrm);

    /// Set TCP connection associated with this wire message
    void SetConnection(TCPIOConnection *p_connection)
    {
        m_p_tcp_connection = p_connection;
    }

    /// Return TCP connection associated with this wire message
    TCPIOConnection* GetConnection()
    {
        return m_p_tcp_connection;
    }

    void SetBufferContainer(uint8_t *p_buffer, uint32_t size)
    {
       m_PackedMsgBuffer = p_buffer;
       m_PackedMsgBufferLength = size;
    }

    uint8_t* m_PackedMsgBuffer;
    uint32_t  m_PackedMsgBufferLength;
    bool m_BufferValid;

    int m_ApplicationTag;                    //<< Application tag associated with this message
    TCPIOConnection *m_p_tcp_connection;     //<< TCP IP connection associated with the message
};

typedef std::shared_ptr<WireMsg> WireMsgPtr;
