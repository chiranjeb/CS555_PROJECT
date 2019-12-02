#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/pixel_produce_msg.hpp"
#include "wiremsg/scene_produce_msg.hpp"

class PixelProducer : public Command
{
public:
    PixelProducer(BlockingMsgQPtr pQ, TCPIOConnectionPtr p_clientConnection, TCPIOConnectionPtr p_MasterConnection, uint16_t requestIndex) :
        Command(pQ), m_p_clientConnection(p_clientConnection), m_pConnectionToMaster(p_MasterConnection), m_requestIndex(requestIndex), m_OutstandingResponsePtr(nullptr)
    {
    }

    /// Set the connection
    void SetConnection(TCPIOConnectionPtr p_connection);

protected:

    /// Run the worker thread
    void ProcessMsg(MsgPtr msg);

    void OnPixelProduceRequestMsg(MsgPtr msg);

    void CheckConnectionAndSendResponse(PixelProduceRequestMsgPtr pRequestMsg, SceneSegmentProduceResponseMsgPtr respMsgPtr);

    void DoCommandComplete(PixelProduceRequestMsgPtr pRequestMsg, SceneSegmentProduceResponseMsgPtr respMsgPtr);

    TCPIOConnectionPtr m_p_clientConnection;
    TCPIOConnectionPtr m_pConnectionToMaster;
    SceneSegmentProduceResponseMsgPtr m_OutstandingResponsePtr;
    PixelProduceRequestMsgPtr m_OutstandingRequestPtr;

    std::mutex m_Mutex;
    uint16_t m_requestIndex;
};

typedef std::shared_ptr<PixelProducer> PixelProducerPtr;
