#include "messaging_framework.hpp"
#include <memory>

/////////////////////////////////////////////////////////////////////////
///
/// Constructor
/// @param q Message Q where the command notification will be posted.
/// 
/////////////////////////////////////////////////////////////////////////
Command::Command(BlockingMsgQPtr q)
{
    m_RequestQ = q;
}

/////////////////////////////////////////////////////////////////////////
///
/// Notify the waiting thread. 
/// @param msg Notification message 
/// 
/////////////////////////////////////////////////////////////////////////
void Command::Notify(MsgPtr msg)
{
    m_RequestQ->Put(MsgQEntry(msg, std::dynamic_pointer_cast<Command>(m_MyLisPtr) ));
}

/////////////////////////////////////////////////////////////////////////
///
/// virtual Process Msg function 
/// @param msg notification message
///  
/////////////////////////////////////////////////////////////////////////
void Command::ProcessMsg(MsgPtr msg)
{
    switch (msg.get()->GetId())
    {
       default:
           break;
    }
}

/////////////////////////////////////////////////////////////////////////
///
/// Notify the waiting thread. 
/// @param msg Notification message 
/// 
/////////////////////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////////////////
///
/// Notify the waiting thread. 
/// @param msgQEntry Notification message 
/// 
/////////////////////////////////////////////////////////////////////////
void MsgQThread::Send(MsgQEntry msgQEntry)
{
    m_RequestQ.get()->Put(msgQEntry);
}



/////////////////////////////////////////////////////////////////////////
///
/// Unhandled message handler
/// @param msg Notification message 
/// 
/////////////////////////////////////////////////////////////////////////
void MsgQThread::ProcessUnHandledMsg(MsgPtr msg)
{
    switch (msg.get()->GetId())
    {
       default:
           break;
    }
}

/////////////////////////////////////////////////////////////////////////
///
/// Worker thread entry
/// @param none
/// 
/////////////////////////////////////////////////////////////////////////
void WorkerThread::Run()
{
    while (true)
    {
        /// Wait for a message to process.
        MsgQEntry entry = m_RequestQ->Take();
        entry.m_Cmd->ProcessMsg(entry.m_Msg);
    }
}

/////////////////////////////////////////////////////////////////////////
///
/// Constructor
/// @param numThreads Nunber of threads
/// @param qD Queue on which all the threads will be waiting on
/// 
/////////////////////////////////////////////////////////////////////////
WorkerThreadList::WorkerThreadList(int numThreads, int qD)
{
    for (int index = 0; index < numThreads; ++index)
    {
        BlockingMsgQPtr msgQPtr = std::make_shared<BlockingQueue<MsgQEntry> >(qD);
        m_RequestQ.push_back(msgQPtr);
        m_WorkerThreads.push_back(new WorkerThread("Worker" + std::to_string(index), msgQPtr));
    }
}

/////////////////////////////////////////////////////////////////////////
///
/// Start all worker threads
/// @param none
/// 
/////////////////////////////////////////////////////////////////////////
void WorkerThreadList::Start()
{
    for (int index = 0; index < m_WorkerThreads.size(); ++index)
    {
        m_WorkerThreads[index]->Start();
    }
}


