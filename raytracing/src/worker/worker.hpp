#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "pixel_producer.hpp"
#include "transport/tcp_io_connection.hpp"

struct WorkerCapabilities
{
    WorkerCapabilities& Get();
    void UpdateAdvertisedHwConcurrencyLevel(int percentageUsedForSceneProduction);
    int m_max_advertised_hw_concurrency_level;
};

class Worker : public MsgQThread
{
public:
    /// Get the Worker
    static Worker& Instance();

    /// Initialize the worker
    static void Initialize(std::string master_address,
                           int master_port,
                           int worker_cmd_processor_q_depth,
                           int max_advertised_hw_concurrency_level,
                           int scene_producer_q_depth,
                           std::string known_scene_name,
                           int known_scene_nx,
                           int known_scene_ny,
                           int known_scene_ns);

    /// Debug routine.
    void Dump();

    /// Return scene descriptor
    SceneDescriptorPtr GetSceneDescriptor(std::size_t sceneId);

    /// Return connection to master.
    TCPIOConnectionPtr GetConnectionToMaster()
    {
        return m_p_ConnectionToMaster;
    }

    /// Make sure we are not being copied around.
    Worker& operator = (Worker& other);

protected:
    /// Worker constructor
    Worker(std::string master_address, int master_port, int worker_cmd_processor_q_depth,
           int numPixelProducers, int pixelProducerQDepth);

    /// Start the worker thread
    void Start();

    /// Run the worker thread
    void Run();

    /// Unsolicited response message handler
    void OnTCPRecvMsg(MsgPtr msg);

    /// Connection establishment response msg
    void OnCreateServerResponse(MsgPtr msg);

    /// Connection establishment response msg
    void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

    /// Worker registration response msg
    void OnWorkerRegistrationRespMsg(MsgPtr msg);

    /// Scene produce request message
    void OnSceneProduceRequestMsg(MsgPtr msg);

    /// Scene produce cleanup message
    void OnSceneProduceCleanupMsg(MsgPtr msg);

    /// Scene produce request message
    void OnPixelProduceRequestMsg(MsgPtr msg);

    /// On scene production completion
    void OnSceneProduceDone(MsgPtr msg);

    /// Determine pixel generation rate based on known scene
    uint32_t DeterminePixelGenerationTimeBasedonKnownScene();

    TCPIOConnectionPtr m_p_ConnectionToMaster;
    std::string m_master_address;
    int m_master_port;
    int m_listening_port;

    std::map<std::size_t, SceneProduceRequestMsgPtr> m_SceneFileMap; /// scene Id to scene description.
    std::map<std::size_t, TCPIOConnectionPtr> m_SceneId2Connection;     /// scene id to connection map.
    std::map<std::string, std::size_t> m_Client2SceneId;             /// Client to tcp connection
    std::map<std::size_t, std::string> m_SceneId2Client;            /// scene id to client map. A scene could be requested by multiple client

    std::multimap<std::size_t, PixelProducerPtr> m_WaitersForConnectionSetup;     /// scene id to connection map.
    std::string m_known_scene_name;
    int m_known_scene_nx;
    int m_known_scene_ny;
    int m_known_scene_ns;
    WorkerThreadList m_PixelProducerThreads;
    uint32_t m_PixelProductionTimeForKnownScene;
};
