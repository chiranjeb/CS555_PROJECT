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
      static Worker s_Worker;
      return s_Worker;
   }

   /// Start the worker thread
   void Start()
   {
      m_thread = new std::thread(&Client::Run, *this);
   }

   /// setup scene name.
   void SetupSceneName(std::string scene_name)
   {
      m_scene_name = scene_name;
   }

   /// Setup master info
   void SetupMasterInfo(std::string master_address, int master_port)
   {
      m_master_address = master_address;
      m_master_port = master_port;
   }

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


   // scene segment response
   std::set<MsgPtr> m_SceneSegmentResponseSet;
   uint32_t m_CurrentPixelToWrite;
   uint32_t m_SceneSizeInPixels;

   // file stream pointer
   std::ifstream *m_file_stream_pointer;
};
