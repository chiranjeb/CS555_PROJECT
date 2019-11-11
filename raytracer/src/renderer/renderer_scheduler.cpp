#include <iostream>
#include "renderer_scheduler.hpp"

void RendererScheduler::Run()
{
   std::cerr << "Started Renderer Scheduler thread" << std::endl;
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
         switch (msgQEntry.m_Msg.get()->GetMsgId())
         {
            default:
               break;
         }
      }
   }
}


