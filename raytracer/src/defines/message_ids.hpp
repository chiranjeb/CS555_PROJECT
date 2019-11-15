#pragma once

enum MsgIds
{
   MsgIdWorkerRegistrationRequest  = 0x01,
   MsgIdWorkerRegistrationResponse = 0x02,

   MsgIdConnectionEstablishmentResponse,
   MsgIdTCPSend,
   MsgIdTCPRecv,
   MsgIdXmitStatus,
   MsgIdTCPShutDownSender,
   MsgIdServerConstructResponse,
};
