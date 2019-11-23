#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "pixel_producer.hpp"

class TCPIOConnection;
class Worker : public MsgQThread
{
   static const int WORKER_CMD_PROCESSOR_MSG_Q_DEPTH = 128;
public:
   Worker() : MsgQThread("Worker", WORKER_CMD_PROCESSOR_MSG_Q_DEPTH)
   {
   }

   /// Get the Worker
   static Worker& Instance()
   {
      static Worker s_Worker;
      return s_Worker;
   }

   /// Start the worker thread
   void Start()
   {
      m_thread = new std::thread(&Worker::Run, *this);
   }

   /// Setup master info
   void SetupMasterInfo(std::string master_address, int master_port)
   {
      m_master_address = master_address;
      m_master_port = master_port;
   }

   /// Return scene descriptor
   SceneDescriptorPtr GetSceneDescriptor(std::size_t sceneId)
   {
      return m_SceneFileMap[sceneId]->GetSceneDescriptor();
   }


   TCPIOConnection* GetConnectionToMaster()
   {
      return m_p_ConnectionToMaster;
   }

protected:

   /// Run the worker thread
   void Run();


   void OnTCPRecvMsg(MsgPtr msg);

   /// Connection establishment response msg
   void OnCreateServerResponse(MsgPtr msg);

   /// Connection establishment response msg
   void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

   /// Worker registration response msg
   void OnWorkerRegistrationRespMsg(MsgPtr msg);

   /// Scene produce request message
   void OnSceneProduceRequestMsg(MsgPtr msg);

   /// Scene produce request message
   void OnPixelProduceRequestMsg(MsgPtr msg);


   void OnSceneProduceDone(MsgPtr msg);

   TCPIOConnection *m_p_ConnectionToMaster;
   std::string m_master_address;
   int m_master_port;
   int m_listening_port;

   std::map<std::size_t, SceneProduceRequestMsgPtr> m_SceneFileMap; /// scene Id to scene description.
   std::map<std::size_t, TCPIOConnection *> m_SceneId2Connection;     /// scene id to connection map.
   std::map<std::string, std::size_t> m_Client2SceneId;             /// Client to tcp connection
   std::map<std::size_t, std::string> m_SceneId2Client;            /// scene id to client map. A scene could be requested by multiple client

   std::map<std::size_t, PixelProducerPtr> m_WaitersForConnectionSetup;     /// scene id to connection map.

};
