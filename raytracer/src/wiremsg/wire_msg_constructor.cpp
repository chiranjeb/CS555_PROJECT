#include "wire_msg.hpp"

///////////////////////////////////////////////////////////////////////////////////////
///    Constructs a message based on a byte array.
///
///    @param data byte array
///    @param dataLength Length of the byte array
///
///    @return Returns the Wire message ptr
///
///////////////////////////////////////////////////////////////////////////////////////
public static WireMsgPtr WireMsgFactory::ConstructMsg(char *buffer, int dataLength)
{
   WireMsgPtr wireMsg(null);

   PreAllocatedStreamBuffer streambuffer(buffer, dataLength);
   std::istream istrm(&streambuffer);

   int msgId;
   istrm >> msgId;

   switch (msgId)
   {
      case MsgIdWorkerRegistrationRequest:
         wireMsg = new PeerNodeRegistrationWireMsg();
         break;
      default:
         //throw new IOException("WireMsg:ConstructMsg - Invalid MsgId" + msgId);
   }
   wireMsg.get()->Unpack(din);
   return wireMsg;
}
