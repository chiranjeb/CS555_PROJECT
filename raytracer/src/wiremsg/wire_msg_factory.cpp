#include "wire_msg_factory.hpp"
#include "worker_registration_msg.hpp"


///////////////////////////////////////////////////////////////////////////////////////
///    Constructs a message based on a byte array.
///
///    @param data byte array
///    @param dataLength Length of the byte array
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

    std::cerr << "WireMsgFactory::ReceivedMsg:" <<  msgId << std::endl;

    switch (msgId)
    {
        case MsgIdWorkerRegistrationRequest:
            wireMsg = WireMsgPtr(new WorkerRegistrationMsg());
            break;
        default:
            //throw new IOException("WireMsg:ConstructMsg - Invalid MsgId" + msgId);
            break;
    }
    wireMsg.get()->Unpack(istrm);
    return wireMsg;
}
