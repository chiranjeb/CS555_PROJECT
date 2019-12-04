#pragma once

////////////////////////////////////////////////////////////////////////////////////////
///
/// Message Id definitions.
///
///////////////////////////////////////////////////////////////////////////////////////
enum MsgIds
{
    /////////// WIRE Messages ////////////////////////////
    MsgIdWorkerRegistrationRequest   = 0x01,
    MsgIdWorkerRegistrationResponse  = 0x02,
    MsgIdSceneProduceRequest         = 0x03,
    MsgIdSceneSegmentProduceResponse = 0x04,
    MsgIdSceneProduceRequestAck      = 0x05,
    MsgIdPixelProduceRequest         = 0x07,
    MsgIdPixelProduceResponse        = 0x08,
    MsgIdSceneProduceCleanup         = 0x09,



    ///////////////// Non Wire messages //////////////////
    MsgIdConnectionEstablishmentResponse,
    MsgIdTCPSend,
    MsgIdTCPRecv,
    MsgIdXmitStatus,
    MsgIdTCPShutDownSender,
    MsgIdTCPConnectionException,
    MsgIdServerConstructResponse,
    MsgIdSceneFileCloseRequest,
    MsgIdSceneFileCloseResponse,
};
