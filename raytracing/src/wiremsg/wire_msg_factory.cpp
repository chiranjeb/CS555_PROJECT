#include "wire_msg_factory.hpp"
#include "worker_registration_msg.hpp"
#include "scene_produce_msg.hpp"
#include "pixel_produce_msg.hpp"
#include <memory>


///////////////////////////////////////////////////////////////////////////////////////
///    Constructs a message based on a byte array.
///
///    @param [buffer] data byte array
///    @param [dataLength] dataLength Length of the byte array
///
///    @return Returns the Wire message ptr
///
///////////////////////////////////////////////////////////////////////////////////////
WireMsgPtr WireMsgFactory::ConstructMsg(uint8_t *buffer, int dataLength)
{
    WireMsgPtr wireMsg(nullptr);

    PreAllocatedStreamBuffer streambuffer(reinterpret_cast<char *>(buffer), dataLength);
    streambuffer.Setg(dataLength);
    std::istream istrm(&streambuffer);

    int msgId;
    istrm >> msgId;

    DEBUG_TRACE_WIRE_MSG("WireMsgFactory::ConstructMsg: " <<  msgId);

    switch (msgId)
    {
       case MsgIdWorkerRegistrationRequest:
           wireMsg = std::make_shared<WorkerRegistrationMsg>();
           break;

       case MsgIdWorkerRegistrationResponse:
           wireMsg = std::make_shared<WorkerRegistrationRespMsg>();
           break;

       case MsgIdSceneProduceRequest:
           wireMsg = std::make_shared<SceneProduceRequestMsg>();
           break;

       case MsgIdSceneSegmentProduceResponse:
           wireMsg = std::make_shared<SceneSegmentProduceResponseMsg>();
           break;

       case MsgIdSceneProduceRequestAck:
           wireMsg = std::make_shared<SceneProduceRequestAckMsg>();
           break;

       case MsgIdPixelProduceRequest:
           wireMsg = std::make_shared<PixelProduceRequestMsg>();
           break;

       case MsgIdPixelProduceResponse:
           wireMsg = std::make_shared<PixelProduceResponseMsg>();
           break;

       case MsgIdSceneProduceCleanup:
           wireMsg = std::make_shared<SceneProduceCleanupMsg>();
           break;

       default:
           wireMsg = WireMsgPtr(nullptr); // Need to think about a better way to handle this.
           break;
    }
    wireMsg->Unpack(istrm);
    //wireMsg.get()->Dump();
    return wireMsg;
}
