#include <iostream>
#include "worker.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include "wiremsg/pixel_produce_msg.hpp"

namespace
{
Worker *g_pWorker;
WorkerCapabilities g_WorkerCapabilities;
}

WorkerCapabilities& WorkerCapabilities::Get()
{
    return g_WorkerCapabilities;
}


void WorkerCapabilities::UpdateAdvertisedHwConcurrencyLevel(int percentageUsedForSceneProduction)
{
    m_max_advertised_hw_concurrency_level = (std::thread::hardware_concurrency() * percentageUsedForSceneProduction) / 100;
    if (m_max_advertised_hw_concurrency_level == 0)
    {
        //We need to have at least one scene production thread.
        m_max_advertised_hw_concurrency_level = 1;
    }

    RELEASE_TRACE("Number of scene producers: " << m_max_advertised_hw_concurrency_level);
}


void Worker::Initialize(std::string master_address,
                        int master_port,
                        int worker_cmd_processor_q_depth,
                        int max_advertised_hw_concurrency_level,
                        int scene_producer_q_depth,
                        std::string known_scene_name,
                        int known_scene_nx,
                        int known_scene_ny,
                        int known_scene_ns)
{

    /// define number of scene producer as number of he threads.
    g_WorkerCapabilities.UpdateAdvertisedHwConcurrencyLevel(max_advertised_hw_concurrency_level);

    /// Start the scene producers.
    int numThreads = g_WorkerCapabilities.m_max_advertised_hw_concurrency_level;

    /// Create and start the worker thread.
    g_pWorker = new Worker(master_address, master_port, worker_cmd_processor_q_depth,
                           numThreads, scene_producer_q_depth);

    g_pWorker->m_known_scene_name = known_scene_name;
    g_pWorker->m_known_scene_nx = known_scene_nx;
    g_pWorker->m_known_scene_ny = known_scene_ny;
    g_pWorker->m_known_scene_ns = known_scene_ns;


    g_pWorker->Start();



    RELEASE_TRACE("Successfully initialized worker.");
}


Worker::Worker(std::string master_address, int master_port, int workerCmdProcessorQDepth,
               int numPixelProducers, int pixelProducerQDepth) :
    MsgQThread("Worker", workerCmdProcessorQDepth),
    m_PixelProducerThreads(numPixelProducers, pixelProducerQDepth)
{
    DEBUG_TRACE("Worker::Worker");
    m_master_address = master_address;
    m_master_port = master_port;
}

/// Get the Worker
Worker& Worker::Instance()
{
    return *g_pWorker;
}

/// Start the worker thread
void Worker::Start()
{
    m_PixelProducerThreads.Start();
    m_thread = new std::thread(&Worker::Run, &Worker::Instance());
}

void Worker::Dump()
{
    DEBUG_TRACE("Dump::Start(this:" << std::hex << this << ")");
    for (std::map<std::size_t, SceneProduceRequestMsgPtr>::iterator iter = m_SceneFileMap.begin(); iter != m_SceneFileMap.end(); ++iter)
    {
        DEBUG_TRACE("SceneProduceRequestMsgPtr: " << iter->first << " " << iter->second.get());
    }

    for (std::map<std::size_t, TCPIOConnectionPtr>::iterator iter = m_SceneId2Connection.begin(); iter != m_SceneId2Connection.end(); ++iter)
    {
        DEBUG_TRACE("m_SceneId2Connection: " << iter->first << " " << iter->second);
    }

    for (std::map<std::string, std::size_t>::iterator iter = m_Client2SceneId.begin(); iter != m_Client2SceneId.end(); ++iter)
    {
        DEBUG_TRACE("m_Client2SceneId: " << iter->first << " " << iter->second);
    }
    
    DEBUG_TRACE("Dump::End");
}


/// Return scene descriptor
SceneDescriptorPtr Worker::GetSceneDescriptor(std::size_t sceneId)
{
    Dump();
    return m_SceneFileMap[sceneId]->GetSceneDescriptor();
}




void Worker::Run()
{
    DEBUG_TRACE("Dump::Run(this:" << std::hex << this << ")");
    while (1)
    {
        MsgQEntry msgQEntry = TakeNext();
        RELEASE_TRACE("Worker::Run("<< std::hex << this << ") - Received MsgId: " << msgQEntry.m_Msg->GetId() << ", Cmd: " << msgQEntry.m_Cmd.get());
        MsgPtr msgPtr = msgQEntry.m_Msg;
        if (msgQEntry.m_Cmd.get() != nullptr)
        {
            msgQEntry.m_Cmd.get()->ProcessMsg(msgQEntry.m_Msg);
        }
        else
        {
            switch (msgQEntry.m_Msg.get()->GetId())
            {
               case MsgIdServerConstructResponse:
                   {
                       OnCreateServerResponse(msgQEntry.m_Msg);
                       break;
                   }
               case MsgIdConnectionEstablishmentResponse:
                   {
                       OnConnectionEstablishmentResponseMsg(msgQEntry.m_Msg);
                       break;
                   }
               case MsgIdWorkerRegistrationResponse:
                   {
                       OnWorkerRegistrationRespMsg(msgQEntry.m_Msg);
                       break;
                   }

               case MsgIdTCPRecv:
                   {
                       OnTCPRecvMsg(msgQEntry.m_Msg);
                       break;
                   }

               default:
                   break;
            }
        }
    }
}

void Worker::OnTCPRecvMsg(MsgPtr msg)
{
    TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr.get()->GetId())
    {
       case MsgIdSceneProduceRequest:
           {
               OnSceneProduceRequestMsg(wireMsgPtr);
               break;
           }

       case MsgIdPixelProduceRequest:
           {
               OnPixelProduceRequestMsg(wireMsgPtr);
               break;
           }

       case MsgIdSceneProduceCleanup:
           {
               OnSceneProduceCleanupMsg(wireMsgPtr);
               break;
           }
       default:
           break;
    }
}

uint32_t Worker::DeterminePixelGenerationTimeBasedonKnownScene()
{
    RELEASE_TRACE("Pixel generation Start");
    int numPixels = m_known_scene_nx * m_known_scene_ny;
    SceneDescriptorPtr sceneDescriptorPtr = SceneFactory::GetScene(m_known_scene_name, m_known_scene_nx, m_known_scene_ny, m_known_scene_ns);

    /// Produce pixels
    char *p_buffer = (char *)malloc(sizeof(char) * numPixels);
    PreAllocatedStreamBuffer streambuff(p_buffer, numPixels);
    std::ostream ostrm(&streambuff);

    auto start_time = std::chrono::system_clock::now();
    ProducePixels(m_known_scene_ny, 0, m_known_scene_nx, 0, sceneDescriptorPtr, ostrm);
    RELEASE_TRACE("Pixel generation End");
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
    RELEASE_TRACE("Pixel generation End:"  << elapsed.count());
    
    free(p_buffer);

    float elapsedTime = (float)elapsed.count()/1000;
    return (elapsedTime < 1) ? 1: int(elapsedTime);
}

void Worker::OnCreateServerResponse(MsgPtr msg)
{
    DEBUG_TRACE("Worker::OnCreateServerResponse(this:" << std::hex << this << ")");
    TCPServerConstructStatusMsg *p_responseMsg =  static_cast<TCPServerConstructStatusMsg *>(msg.get());
    m_listening_port  = p_responseMsg->GetPort();

    // Let's establish a connection
    m_p_ConnectionToMaster = nullptr;
    TransportMgr::Instance().EstablishNewConnection(m_master_address, m_master_port, GetThrdListener(), true);

    m_PixelProductionTimeForKnownScene = DeterminePixelGenerationTimeBasedonKnownScene();
}

void Worker::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
    DEBUG_TRACE("Successfully established connection(this:" << std::hex << this << ")");

    TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
    if (m_p_ConnectionToMaster == nullptr)
    {
        m_p_ConnectionToMaster = p_responseMsg->GetConnection();
        std::string hostname = TransportMgr::Instance().MyName();

        /// We are expecting response, So allocate an apptag and pass our thread listener which will route back the message to us.
        uint16_t advertisedHwThreadConcurrency  = g_WorkerCapabilities.m_max_advertised_hw_concurrency_level;
        WorkerRegistrationMsgPtr reigstrationMsgPtr =  std::make_shared<WorkerRegistrationMsg>(hostname, m_listening_port, 
                                                                                               advertisedHwThreadConcurrency, 
                                                                                               m_PixelProductionTimeForKnownScene);
        reigstrationMsgPtr.get()->SetAppTag(m_p_ConnectionToMaster->AllocateAppTag());
        m_p_ConnectionToMaster->SendMsg(reigstrationMsgPtr, GetThrdListener());
    }
    else
    {
        UniqueServerId serverId(p_responseMsg->GetServerAddress(), p_responseMsg->GetServerPort());
        std::size_t sceneId = m_Client2SceneId[serverId.toString()];
        m_SceneId2Connection.insert(std::pair<std::size_t, TCPIOConnectionPtr>(sceneId, p_responseMsg->GetConnection()));

        /// Get all the pixel producers waiting for connection setup
        auto it = m_WaitersForConnectionSetup.equal_range(sceneId);
        for (auto itr = it.first; itr != it.second; ++itr)
        {
            /// Update the connection
            itr->second->SetConnection(p_responseMsg->GetConnection());
            //m_WaitersForConnectionSetup[sceneId]->SetConnection(p_responseMsg->GetConnection());
        }
        m_WaitersForConnectionSetup.erase(sceneId);
    }
}

void Worker::OnWorkerRegistrationRespMsg(MsgPtr msg)
{
    DEBUG_TRACE("Worker::OnWorkerRegistrationRespMsg(this:" << std::hex << this << ")");
    WorkerRegistrationRespMsg *p_responseMsg =  static_cast<WorkerRegistrationRespMsg *>(msg.get());
    p_responseMsg->GetConnection()->FreeAppTag(p_responseMsg->GetAppTag());
}

void Worker::OnSceneProduceRequestMsg(MsgPtr msg)
{
    DEBUG_TRACE("Worker::OnSceneProduceRequestMsg(this:" << std::hex << this << ")");

    /// save the scene id.
    SceneProduceRequestMsgPtr requestMsgPtr  = std::dynamic_pointer_cast<SceneProduceRequestMsg>(msg);

    /// Update scene file map
    DEBUG_TRACE("Worker::Update scene file map");
    m_SceneFileMap.insert(std::pair<std::size_t, SceneProduceRequestMsgPtr>(requestMsgPtr->GetSceneId(), requestMsgPtr));

    DEBUG_TRACE("Worker::Update scene to client id");
    /// Scene to client for forward look up
    UniqueServerId serverId = UniqueServerId(requestMsgPtr->GetClientAddress(), requestMsgPtr->GetClientPort());
    m_SceneId2Client.insert(std::pair<std::size_t, std::string>(requestMsgPtr->GetSceneId(), serverId.toString()));

    DEBUG_TRACE("scene Id: " << requestMsgPtr->GetSceneId() << "client: " << serverId.toString() << std::endl);

    DEBUG_TRACE("Worker::Update m_Client2SceneId");
    /// Client to scene look up
    m_Client2SceneId.insert(std::pair<std::string, std::size_t>(serverId.toString(), requestMsgPtr->GetSceneId()));

    Dump();

    /// Establishes a connection with the client
    TransportMgr::Instance().EstablishNewConnection(requestMsgPtr->GetClientAddress(), requestMsgPtr->GetClientPort(), GetThrdListener(), true);
}

void Worker::OnSceneProduceCleanupMsg(MsgPtr msg)
{
    SceneProduceCleanupMsgPtr requestMsgPtr  = std::dynamic_pointer_cast<SceneProduceCleanupMsg>(msg);
    RELEASE_TRACE("Worker::OnSceneProduceCleanupMsg(this:" << std::hex << this << ") - Cleaning up scene artifacts, scene Id:" << requestMsgPtr->GetSceneId());
    auto iter = m_SceneFileMap.find(requestMsgPtr->GetSceneId());
    if (iter != m_SceneFileMap.end())
    {
        m_SceneFileMap.erase(requestMsgPtr->GetSceneId());
        m_Client2SceneId.erase(m_SceneId2Client[requestMsgPtr->GetSceneId()]);
        m_SceneId2Client.erase(requestMsgPtr->GetSceneId());
        m_SceneId2Connection.erase(requestMsgPtr->GetSceneId());
    }
    Dump();

}

void Worker::OnPixelProduceRequestMsg(MsgPtr msg)
{
    /// Now we will start producing pixels for which we have already established all the contexts.
    PixelProduceRequestMsgPtr requestMsgPtr  = std::dynamic_pointer_cast<PixelProduceRequestMsg>(msg);
    auto it = m_SceneId2Connection.find(requestMsgPtr->GetSceneId());    
    TCPIOConnectionPtr pConnection = (it == m_SceneId2Connection.end()) ? nullptr: it->second ;
    for (int index = 0; index < requestMsgPtr->GetNumRequests(); ++index)
    {
        PixelProduceRequest *pRequest = requestMsgPtr->GetRequest(index);
        PixelProducerPtr cmdPtr = std::make_shared<PixelProducer>(m_PixelProducerThreads.GetListeningQ(pRequest->m_PipelineId), pConnection, m_p_ConnectionToMaster, index);
        m_PixelProducerThreads.Send(pRequest->m_PipelineId, MsgQEntry(requestMsgPtr, cmdPtr));
        if (pConnection == nullptr)
        {
            m_WaitersForConnectionSetup.insert(std::pair<std::size_t, PixelProducerPtr>(requestMsgPtr->GetSceneId(), cmdPtr));
        }
    }
}




