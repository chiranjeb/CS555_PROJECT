#pragma once

enum MsgIds
{
   MsgIdWorkerRegistrationRequest  = 0x01,
   MsgIdWorkerRegistrationResponse = 0x02,
   MsgIdSceneDistributionMsg       = 0x03,

   MsgIdConnectionEstablishmentResponse,
   MsgIdTCPSend,
   MsgIdTCPRecv,
   MsgIdXmitStatus,
   MsgIdTCPShutDownSender,
   MsgIdServerConstructResponse,
};
