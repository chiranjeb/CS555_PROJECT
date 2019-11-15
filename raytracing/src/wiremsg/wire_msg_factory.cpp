#include "wire_msg_factory.hpp"
#include "worker_registration_msg.hpp"


///////////////////////////////////////////////////////////////////////////////////////
///    Constructs a message based on a byte array.
///
///    @param [buffer] data byte array
///    @param [dataLength] dataLength Length of the byte array
///
///    @return Returns the Wire message ptr
///
///////////////////////////////////////////////////////////////////////////////////////
WireMsgPtr WireMsgFactory::ConstructMsg(char *buffer, int dataLength)
{
    WireMsgPtr wireMsg(nullptr);

    PreAllocatedStreamBuffer streambuffer(buffer, dataLength);
    streambuffer.Setg(dataLength);
    std::istream istrm(&streambuffer);

    int msgId;
    istrm >> msgId;

    DEBUG_TRACE("WireMsgFactory::ConstructMsg: " <<  msgId);

    switch (msgId)
    {
        case MsgIdWorkerRegistrationRequest:
            wireMsg = WireMsgPtr(new WorkerRegistrationMsg());
            break;

        case MsgIdWorkerRegistrationResponse:
            wireMsg = WireMsgPtr(new WorkerRegistrationRespMsg());
            break;

        default:
            wireMsg = WireMsgPtr(nullptr); // Need to think about a better way to handle this.
            break;
    }
    wireMsg.get()->Unpack(istrm);
    return wireMsg;
}
