#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include <fstream>

class TCPIOConnection;
class Client : public MsgQThread
{
   static const int WORKER_CMD_PROCESSOR_MSG_Q_DEPTH = 128;
public:
   Client() : MsgQThread("Client", WORKER_CMD_PROCESSOR_MSG_Q_DEPTH)
   {
   }

   /// Get the Worker
   static Client& Instance()
   {
      static Client s_Client;
      return s_Client;
   }

   /// Start the worker thread
   void Start();

   /// setup scene name.
   void SetupSceneName(std::string scene_name);

   /// Setup master info
   void SetupMasterInfo(std::string master_address, int master_port);

protected:

   /// Run the worker thread
   void Run();

   /// Connection establishment response msg
   void OnCreateServerResponse(MsgPtr msg);

   /// Connection establishment response msg
   void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

   /// Scene produce request ack message
   void OnSceneProduceRequestAckMsg(MsgPtr msg);

   /// Scene segment produce response message
   void OnSceneSegmentProduceRespMsg(MsgPtr msg);

   TCPIOConnection *m_p_ConnectionToMaster;
   std::string m_master_address;
   int m_master_port;
   int m_listening_port;
   std::string m_scene_name;


   uint32_t m_CurrentPixelToWrite;
   uint32_t m_SceneSizeInPixels;


   // scene segment response
   std::set<MsgPtr> m_SceneSegmentResponseSet;

};
