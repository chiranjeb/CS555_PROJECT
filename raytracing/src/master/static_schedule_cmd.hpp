#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include<map>

class TCPIOConnection;
class StaticScheduleCmd : public Command
{
public:
   StaticScheduleCmd(std::vector<std::string>& workerList, BlockingMsgQPtr pQ) : Command(pQ), m_workerList(workerList)
   {
   }

   /// Actual Scheduler thread
   void ProcessMsg(MsgPtr msg);
protected:

   /// Scene produce request message handler.
   void OnSceneProduceRequestMsg(MsgPtr msg);

   /// Pixel produce response message.
   void OnPixelProduceResponseMsg(MsgPtr msg);


   /// Different chunking of the work.
   void GenerateSequentialPixelWorkload(std::size_t sceneId);

   uint32_t  m_NX, m_NY;
   uint32_t m_workloadInPixels;
   uint32_t m_NumPendingCompletionResponse;
   std::map<std::string, bool> m_workOrder;
   TCPIOConnection *m_p_client_connection;
   std::vector<std::string> m_workerList;
};

typedef std::shared_ptr<StaticScheduleCmd> StaticScheduleCmdPtr;
