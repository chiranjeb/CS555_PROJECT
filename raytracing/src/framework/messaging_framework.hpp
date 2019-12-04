#pragma once
#include "message.hpp"
#include "blocking_queue.hpp"
#include <thread>
#include <memory>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////
///
/// Classes responsible for messaging and threading framework:
///
///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///
/// Listener - The observer interface.
///
///////////////////////////////////////////////////////////////////////////////////////
class Listener
{
public:
    virtual void Notify(MsgPtr msg) = 0;
};

typedef std::shared_ptr<Listener> ListenerPtr;

class MsgQThread;


////////////////////////////////////////////////////////////////////////////////////////
///
/// CommandBase - An interface that extends the observer interface.
///
///////////////////////////////////////////////////////////////////////////////////////
class CommandBase : public Listener
{

public:
    virtual void Notify(MsgPtr msg) = 0;
    virtual void ProcessMsg(MsgPtr msg) = 0;
};
typedef std::shared_ptr<CommandBase> CommandBasePtr;

////////////////////////////////////////////////////////////////////////////////////////
///
///  MsgQEntry -  A pair of Command based and message. Primariliy used as a communication message between threads.
///
///////////////////////////////////////////////////////////////////////////////////////
class MsgQEntry
{
public:
    /// This class defines a message queue entry
    MsgQEntry(MsgPtr msg, CommandBasePtr cmd)
    {
        m_Msg = msg;
        m_Cmd = cmd;
    }

    /// Response message to be sent to a command.
    MsgPtr  m_Msg;

    /// Command where the response message needs to be routed to.
    /// A null command needs to be handled generic way. One example
    /// could be  - an unsolicited message being put in a queue of
    /// message queue entry with a value of null command.
    CommandBasePtr m_Cmd;
};
typedef std::shared_ptr<BlockingQueue<MsgQEntry> > BlockingMsgQPtr;

////////////////////////////////////////////////////////////////////////////////////////
///
///  Command - Base implementation for all the commands. 
///
///////////////////////////////////////////////////////////////////////////////////////
class Command : public CommandBase
{
public:
    /// Constructor
    Command(BlockingMsgQPtr q);

    /// over-ridden Notify
    virtual void Notify(MsgPtr msg);

    /// Process Msg
    virtual void ProcessMsg(MsgPtr msg);

    /// Save memento
    void SaveMemento(ListenerPtr context)
    {
        m_MyLisPtr = context;
    }

protected:
    ListenerPtr        m_MyLisPtr;
    BlockingMsgQPtr    m_RequestQ;
};
typedef std::shared_ptr<Command> CommandPtr;


////////////////////////////////////////////////////////////////////////////////////////
///
///  MsgQListener -  A type of listener that can be notified to send a message other thread using a blocking Q.
///
///////////////////////////////////////////////////////////////////////////////////////
class MsgQListener : public Listener
{
public:
    /// Constructor
    MsgQListener(BlockingMsgQPtr msgQ)
    {
        m_MsgQ = msgQ;
    }

    /// Notify the waiting thread.
    virtual void Notify(MsgPtr msgPtr);

private:
    BlockingMsgQPtr   m_MsgQ;
};
typedef std::shared_ptr<MsgQListener> MsgQListenerPtr;

////////////////////////////////////////////////////////////////////////////////////////
///
///  Thread -  Thread wrapper.
///
///////////////////////////////////////////////////////////////////////////////////////
class Thread
{
public:
    Thread(std::string threadName = "unknown")
    {
        m_ThreadName = threadName;
    }

protected:
    std::string  m_ThreadName;
    std::thread *m_thread;
};

////////////////////////////////////////////////////////////////////////////////////////
///
///  MsgQThread - A thread that listens to a message Q.
///
///////////////////////////////////////////////////////////////////////////////////////
class MsgQThread : public Thread
{
public:
    /// This class defines a message queue entry
    MsgQThread(std::string threadName, int threadMsgQDepth);

    /// Send a message to a thread
    void Send(MsgQEntry msgQEntry);

    /// Get the thread listener
    ListenerPtr GetThrdListener()
    {
        return m_ThrdLis;
    }

    /// Return listening q
    BlockingMsgQPtr GetListeningQ()
    {
        return m_RequestQ;
    }

    /// Return the next element
    MsgQEntry TakeNext()
    {
        return m_RequestQ->Take();
    }

    /// Unhandled message
    void ProcessUnHandledMsg(MsgPtr msg);

protected:
    /// Blocking Queue is going to be waiting for
    BlockingMsgQPtr    m_RequestQ;

    MsgQListenerPtr m_ThrdLis;
};


////////////////////////////////////////////////////////////////////////////////////////
///
///  MsgQThreadPoolLis -  A listener for multiple producer and multiple MsgThread as a consumer.
///
///////////////////////////////////////////////////////////////////////////////////////
template<class T>
class MsgQThreadPoolLis : public Listener
{
public:
    /// This class defines a message queue entry
    MsgQThreadPoolLis(T **threadPoolPtr = nullptr, int N = 0)
    {
        Construct(threadPoolPtr, N);
    }

    /// Construct a pool.
    void Construct(T **threadPoolPtr, int N)
    {
        m_ThreadsPtr = threadPoolPtr;
        m_NextThread = 0;
        m_NumThreads = N;
    }

    /// Sends notifucation. One of the free thread will be pulling the message from the Q.
    virtual void Notify(MsgPtr msg)
    {
        std::mutex m_Mutex;
        if (m_NextThread >= m_NumThreads)
        {
            m_NextThread = 0;
        }
        m_ThreadsPtr[m_NextThread++]->Send(MsgQEntry(msg, nullptr));
    }

protected:
    T   **m_ThreadsPtr;
    int m_NumThreads;
    int m_NextThread;
};


////////////////////////////////////////////////////////////////////////////////////////
///
///  WorkerThread - Worker thread
///
///////////////////////////////////////////////////////////////////////////////////////
class WorkerThread : public Thread
{
public:
    BlockingMsgQPtr  m_RequestQ;

    /// Constructor
    WorkerThread(std::string threadname, BlockingMsgQPtr  requestQ) : Thread(threadname)
    {
        m_RequestQ = requestQ;
    }

    /// Start the scheduler thread
    void Start()
    {
        m_thread = new std::thread(&WorkerThread::Run, *this);
    }

private:
    /// Actual Scheduler thread
    void Run();
};

////////////////////////////////////////////////////////////////////////////////////////
///
///  WorkerThreadList -  List of worker threads.
///
///////////////////////////////////////////////////////////////////////////////////////
class WorkerThreadList
{
public:
    /// Constructor
    WorkerThreadList(int numThreads, int qD);

    /// Start all worker threads
    void Start();

    /// Send message to a thread
    void Send(int threadId, MsgQEntry msgQEntry)
    {
        m_RequestQ[threadId]->Put(msgQEntry);
    }

    /// Return the reference to the listening queue.
    BlockingMsgQPtr GetListeningQ(int threadId)
    {
        return m_RequestQ[threadId];
    }

protected :
    /// Request q on which all the threads will be waiting on.
    std::vector<BlockingMsgQPtr> m_RequestQ;

    /// Pointer to all the worker threads.
    std::vector<WorkerThread *>  m_WorkerThreads;
};











