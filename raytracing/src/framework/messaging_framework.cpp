#include "messaging_framework.hpp"
#include <memory>

Command::Command(MsgQThread *thread)
{
   m_pHandlerThreadPtr = thread;
}

Listener* Command::GetHandlerThrdListener()
{
   return m_pHandlerThreadPtr->GetThrdListener();
}

void Command::Notify(MsgPtr msg)
{
   //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, TraceLogger.MODULE_FRAMEWORK, "Command::Notify");
   m_pHandlerThreadPtr->Send(MsgQEntry(msg, CommandPtr(this)));
}

/** 
* virtual Process Msg function 
* @param msg notification message
*  
*/
void Command::ProcessMsg(MsgPtr msg)
{
   switch (msg.get()->GetId())
   {
   default:
      //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, "Cmd Encountered - Unhandled Message:" + msg.GetId());
      break;
   }
}
/**
* Notify the waiting thread. 
* @param msg Notification message 
*/
void MsgQThreadListener::Notify(MsgPtr msgPtr)
{
   m_MsgQ.get()->Put(MsgQEntry(msgPtr, CommandPtr(nullptr)));
}



/// This class defines a message queue entry
MsgQThread::MsgQThread(std::string threadName, int threadMsgQDepth) :  Thread(threadName),
   m_RequestQ(new  BlockingQueue<MsgQEntry>(threadMsgQDepth)),
   m_ThrdLis(new MsgQThreadListener(m_RequestQ)) 
{
}

void MsgQThread::Send(MsgQEntry msgQEntry)
{
   m_RequestQ.get()->Put(msgQEntry);
}


void MsgQThread::ProcessUnHandledMsg(MsgPtr msg)
{
   switch (msg.get()->GetId())
   {
   default:
      //TraceLogger.Instance().Println(TraceLogger.LEVEL_DEBUG, "MsgQThread::ProcessUnHandledMsg  - Encountered - Unhandled Message:" + msg.GetId());
      break;
   }
}
