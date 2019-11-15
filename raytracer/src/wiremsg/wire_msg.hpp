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
    std::pair<char *, int> GetPackedBytes(char *pre_allocated_buffer, int size);

    ///  Custom Message serializer
    void Pack(std::ostream &ostrm);

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

    int m_ApplicationTag;                    //<< Application tag associated with this message
    TCPIOConnection *m_p_tcp_connection;     //<< TCP IP connection associated with the message
};

typedef std::shared_ptr<WireMsg> WireMsgPtr;
