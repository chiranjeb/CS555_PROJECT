#include "messaging_framework.hpp"
#include <memory>

Command::Command(BlockingMsgQPtr q)
{
   m_RequestQ = q;
}


void Command::Notify(MsgPtr msg)
{
   m_RequestQ->Put(MsgQEntry(msg, CommandPtr(this)));
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
         break;
   }
}
/**
* Notify the waiting thread. 
* @param msg Notification message 
*/
void MsgQListener::Notify(MsgPtr msgPtr)
{
   m_MsgQ.get()->Put(MsgQEntry(msgPtr, CommandPtr(nullptr)));
}



/// This class defines a message queue entry
MsgQThread::MsgQThread(std::string threadName, int threadMsgQDepth) :  Thread(threadName),
   m_RequestQ(new  BlockingQueue<MsgQEntry>(threadMsgQDepth)),
   m_ThrdLis(new MsgQListener(m_RequestQ))
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




// Actual Scheduler thread
void WorkerThread::Run()
{
   while (true)
   {
      //Wait for a message to process.
      MsgQEntry entry = m_RequestQ->Take();
      entry.m_Cmd->ProcessMsg(entry.m_Msg);
   }
}



ThreadPoolManager::ThreadPoolManager(int numThreads, BlockingMsgQPtr blockingQ)
{
   m_RequestQ = blockingQ;
   for (int index = 0; index < numThreads; ++index)
   {
      //Create a worker thread.
      m_WorkerThreads.push_back(new WorkerThread("WORKER-THREAD-" + std::to_string(index) + ": ", m_RequestQ));
   }
}

void ThreadPoolManager::Start()
{
   for (int index = 0; index < m_WorkerThreads.size(); ++index)
   {
      m_WorkerThreads[index]->Start();
   }
}

