#pragma once
#include "transport/tcp_io_connection.hpp"
#include "framework/message.hpp"

/**
 * Base class for all wire message.
 * 
 *  
 */
public class WireMsg : public Msg
{

public:
    /// Message constructor
    WireMsg(int msgId) : Msg(msgId)
    {
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
    byte[] GetPackedBytes();

    ///  Custom Message serializer
    void Pack(DataOutputStream out);

    ///  Custom message deserializer
    void Unpack(DataInputStream in) ;

    //// Set TCP connection associated with this wire message
    void SetConnection(TCPConnectionPtr connection)
    {
        m_TCPConnection = connection;
    }

    ///  Return TCP connection associated with this wire message
    TCPConnectionPtr GetConnection()
    {
        return m_TCPConnectionPtr;
    }


    int m_ApplicationTag;               //<< Application tag associated with this message
    TCPConnectionPtr m_TCPConnectionPtr;      //<< TCP IP connection associated with the message
}
