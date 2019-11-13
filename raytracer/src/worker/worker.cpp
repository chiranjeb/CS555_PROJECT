#include <iostream>
#include "worker.hpp"

void Worker::Run()
{
   std::cerr << "Started Worker thread" << std::endl;
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
   std::cerr << "Successfully established connection" << std::endl;
}

