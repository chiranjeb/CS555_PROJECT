#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "scene_writer.hpp"

class TCPIOConnection;
class Client : public MsgQThread
{
public:
    /// Instantiate the Client
    static void Instantiate(std::string master_address, int master_port, int clientThreadQDepth, std::string scene_name);
    
    /// Return the singleton instance of the client.
    static Client& Instance();

protected:
    /// Client constructor
    Client(int clientThreadQDepth) : MsgQThread("Client", clientThreadQDepth)
    {
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

    TCPIOConnection *m_p_ConnectionToMaster;
    std::string m_master_address;
    int m_master_port;
    int m_listening_port;
    std::string m_scene_name;


    uint32_t m_CurrentPixelToWrite;
    uint32_t m_SceneSizeInPixels;

    CommandPtr  m_SceneWriterPtr;
    BlockingMsgQPtr m_SceneWriterMsgQ;

    // scene segment response
    std::set<MsgPtr, SceneSegmentProduceResponseMsgCompare> m_SceneSegmentResponseSet;


    static std::mutex m_Mutex;

};
