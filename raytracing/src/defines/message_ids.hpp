#pragma once

enum MsgIds
{
   MsgIdWorkerRegistrationRequest  = 0x01,
   MsgIdWorkerRegistrationResponse = 0x02,
   MsgIdSceneProduceRequest        = 0x03,
   MsgIdSceneSegmentProduceResponse       = 0x04,
   MsgIdSceneProduceRequestAck     = 0x05,

   MsgIdPixelProduceRequest     = 0x07,
   MsgIdPixelProduceResponse    = 0x08,

   MsgIdConnectionEstablishmentResponse,
   MsgIdTCPSend,
   MsgIdTCPRecv,
   MsgIdXmitStatus,
   MsgIdTCPShutDownSender,
   MsgIdServerConstructResponse,
};
