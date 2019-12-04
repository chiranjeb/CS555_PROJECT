#pragma once
#include <chrono>
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "scene_writer.hpp"

////////////////////////////////////////////////////////////////////////////////////////
///
/// This is the client class responsible for stitching different segments of a scene.
/// It can receive fragments out of order. But it will serialize all the segments and
/// write to the final scene file.
///
///////////////////////////////////////////////////////////////////////////////////////
class Client : public MsgQThread
{
public:
    /// Instantiate the Client
    static void Instantiate(std::string master_address, int master_port, int clientThreadQDepth, std::string scene_name,
            std::uint32_t width, std::uint32_t height, std::uint32_t rpp);
    
    /// Return the singleton instance of the client.
    static Client& Instance();

protected:
    /// Client constructor
    Client(int clientThreadQDepth) : MsgQThread("Client", clientThreadQDepth)
    {
        m_CurrentPixelToWrite = 0;
    }

    /// Start the worker thread
    void Start();

    /// Run the worker thread
    void Run();

    void OnTCPRecvMsg(MsgPtr msg);

    /// Connection establishment response msg
    void OnCreateServerResponse(MsgPtr msg);

    /// Connection establishment response msg
    void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

    /// Scene produce request ack message
    void OnSceneProduceRequestAckMsg(MsgPtr msg);

    /// Scene segment produce response message
    void OnSceneSegmentProduceRespMsg(MsgPtr msg);

    /// Scene file close response.
    void OnSceneFileCloseResponse(MsgPtr msg);

    /// Connection to master
    TCPIOConnectionPtr m_p_ConnectionToMaster;

    /// Master host address
    std::string m_master_address;

    /// Master port
    int m_master_port;

    /// Master listening port
    int m_listening_port;

    /// scene name
    std::string m_scene_name;

    /// scene dimension
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_rpp;

    /// Current write pointer
    uint32_t m_CurrentPixelToWrite;

    /// Scene size in pixels.
    uint32_t m_SceneSizeInPixels;

    /// Scene writer command
    CommandPtr  m_SceneWriterPtr;

    /// Scene writer thread message Q. Client releases fragments to this Q.
    BlockingMsgQPtr m_SceneWriterMsgQ;

    /// scene segment response set sorted based on the offset of the scene.
    std::set<MsgPtr, SceneSegmentProduceResponseMsgCompare> m_SceneSegmentResponseSet;

    /// Mutex to protect Instantiate call from different threads. This is probably
    /// a moot point for this application. But, doing it as a good practice.
    static std::mutex m_Mutex;
};
