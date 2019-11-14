#include <iostream>
#include "worker.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"

void Worker::Run()
{
   RELEASE_TRACE("Started Worker thread");
   while (1)
   {
      MsgQEntry msgQEntry = TakeNext();
      MsgPtr msgPtr = msgQEntry.m_Msg;
      if (msgQEntry.m_Cmd.get() != nullptr)
      {
         msgQEntry.m_Cmd.get()->ProcessMsg(msgQEntry.m_Msg);
      }
      else
      {
         switch (msgQEntry.m_Msg.get()->GetId())
         {
            case MsgIdConnectionEstablishmentResponse:
               OnConnectionEstablishmentResponseMsg(msgQEntry.m_Msg);
               break; 

            default:
               break;
         }
      }
   }
}


void Worker::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
   RELEASE_TRACE("Successfully established connection");
   TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
   m_p_TCPIOConnection = p_responseMsg->GetConnection();
   WorkerRegistrationMsgPtr reigstrationMsgPtr =  WorkerRegistrationMsgPtr(new WorkerRegistrationMsg("Worker1", 40));
   //reigstrationMsgPtr.get()->SetAppTag(m_p_TCPIOConnection->AllocateAppTag());
   m_p_TCPIOConnection->SendMsg(reigstrationMsgPtr, nullptr);
}
