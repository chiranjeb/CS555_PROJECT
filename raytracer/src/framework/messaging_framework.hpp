#pragma once
#include "message.hpp"
#include "blocking_queue.hpp"
#include <thread>
#include <memory>


/// Base Listener class
class Listener
{
public:
   virtual void Notify(MsgPtr msg) = 0;
};

typedef std::shared_ptr<Listener> ListenerPtr;

class MsgQThread;
/// Command class
class Command : public Listener
{
public:
   /// Constructor
   Command(MsgQThread *p_msgQThread);

   /// over-ridden Notify
   virtual void Notify(MsgPtr msg);

   /// Return the thread listener.
   Listener* GetHandlerThrdListener();

   /// Process Msg
   virtual void ProcessMsg(MsgPtr msg);

protected:
   MsgQThread *m_pHandlerThreadPtr;
};


typedef std::shared_ptr<Command> CommandPtr;

/// Message Queue entry
class MsgQEntry
{
public:
   /// This class defines a message queue entry
   MsgQEntry(MsgPtr msg, CommandPtr cmd)
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
   CommandPtr m_Cmd;
};

typedef std::shared_ptr<MsgQEntry> MsgQEntryPtr;
typedef std::shared_ptr<BlockingQueue<MsgQEntry>> BlockingMsgQPtr;

class MsgQThreadListener : public Listener
{
public:
   /// Constructor
   MsgQThreadListener(BlockingMsgQPtr msgQ)
   {
      m_MsgQ = msgQ;
   }

   /// Notify the waiting thread.
   virtual void Notify(MsgPtr msgPtr);

private:
   BlockingMsgQPtr   m_MsgQ;
};


typedef std::shared_ptr<MsgQThreadListener> MsgQThreadListenerPtr;


/// Thread base
class Thread
{
public:
   Thread(std::string threadName="unknown")
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
   ListenerPtr GetThrdListener()
   {
      return m_ThrdLis;
   }

   /// Unhandled message
   void ProcessUnHandledMsg(MsgPtr msg);

protected:
   /// Blocking Queue is going to be waiting for
   BlockingMsgQPtr    m_RequestQ;

   MsgQThreadListenerPtr m_ThrdLis;
};





