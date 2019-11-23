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
       DEBUG_TRACE("Worker::Worker" );
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
      m_thread = new std::thread(&Worker::Run, &Worker::Instance());
   }

   /// Setup master info
   void SetupMasterInfo(std::string master_address, int master_port)
   {
      DEBUG_TRACE("Dump::Start(this:" << std::hex << this << ")");
      m_master_address = master_address;
      m_master_port = master_port;
   }

   void Dump()
   {
       DEBUG_TRACE("Dump::Start(this:" << std::hex << this << ")");
       for (std::map<std::size_t, SceneProduceRequestMsgPtr>::iterator iter = m_SceneFileMap.begin(); iter!=m_SceneFileMap.end(); ++iter)
       {
           DEBUG_TRACE("SceneProduceRequestMsgPtr: " << iter->first << " " << iter->second.get() );
       }
       DEBUG_TRACE("Dump::End" );
   }

   /// Return scene descriptor
   SceneDescriptorPtr GetSceneDescriptor(std::size_t sceneId)
   {
      DEBUG_TRACE("GetSceneDescriptor: " << sceneId );
      Dump();
      SceneProduceRequestMsgPtr request = m_SceneFileMap[sceneId];
      DEBUG_TRACE("SceneProduceRequestMsgPtr: " << std::hex << request.get() );
      SceneDescriptorPtr descript  = request->GetSceneDescriptor();
      DEBUG_TRACE("SceneDescriptorPtr: " << std::hex << descript.get() );
      return descript;
   }


   TCPIOConnection* GetConnectionToMaster()
   {
      return m_p_ConnectionToMaster;
   }

   Worker& operator=(Worker& other);

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
