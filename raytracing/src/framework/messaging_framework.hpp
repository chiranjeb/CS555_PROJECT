#pragma once
#include "message.hpp"
#include "blocking_queue.hpp"
#include <thread>
#include <memory>
#include <vector>


/// Base Listener class
class Listener
{
public:
   virtual void Notify(MsgPtr msg) = 0;
};

typedef std::shared_ptr<Listener> ListenerPtr;

class MsgQThread;

class CommandBase : public Listener
{

public:
   virtual void Notify(MsgPtr msg)=0;
   virtual void ProcessMsg(MsgPtr msg)=0;
};


typedef std::shared_ptr<CommandBase> CommandBasePtr;

/// Message Queue entry
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

/// Command class
class Command : public CommandBase
{
public:
   /// Constructor
   Command(BlockingMsgQPtr q);

   /// over-ridden Notify
   virtual void Notify(MsgPtr msg);

   /// Process Msg
   virtual void ProcessMsg(MsgPtr msg);

protected:
   BlockingMsgQPtr    m_RequestQ;
};


typedef std::shared_ptr<Command> CommandPtr;



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


/// Thread base
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

/// Message Q Thread
class MsgQThread : public Thread
{
public:
   /// This class defines a message queue entry
   MsgQThread(std::string threadName, int threadMsgQDepth);

   /// Send a message to a thread
   void Send(MsgQEntry msgQEntry);

   /// Get the thread listener
   Listener* GetThrdListener()
   {
      return m_ThrdLis.get();
   }

   MsgQEntry TakeNext()
   {
      return m_RequestQ.get()->Take();
   }

   /// Unhandled message
   void ProcessUnHandledMsg(MsgPtr msg);

protected:
   /// Blocking Queue is going to be waiting for
   BlockingMsgQPtr    m_RequestQ;

   MsgQListenerPtr m_ThrdLis;
};




class WorkerThread : public Thread
{
public:
   BlockingMsgQPtr  m_RequestQ;

   // Constructor
   WorkerThread(std::string threadname, BlockingMsgQPtr  requestQ) : Thread(threadname)
   {
      m_RequestQ = requestQ;
   }


   // Start the scheduler thread
   void Start()
   {
      m_thread = new std::thread(&WorkerThread::Run, *this);
   }

private:
   // Actual Scheduler thread
   void Run();
};



class ThreadPoolManager
{
public:
   // This class defines a message queue entry
   ThreadPoolManager(int numThreads, BlockingMsgQPtr blockingQ);

   void Start();

   void Send(MsgQEntry msgQEntry)
   {
      m_RequestQ->Put(msgQEntry);
   }

   BlockingMsgQPtr GetListeningQ()
   {
      return m_RequestQ;
   }

protected :
   BlockingMsgQPtr   m_RequestQ;
   std::vector<WorkerThread *>  m_WorkerThreads;
};







