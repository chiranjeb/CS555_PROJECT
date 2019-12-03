#include <iostream>
#include "client.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "transport/transport_mgr.hpp"
#include <functional>


WorkerThread *g_pWorkerThread;
Client *m_pClient = nullptr;
std::mutex Client::m_Mutex;
std::chrono::time_point<std::chrono::system_clock> start_time;

////////////////////////////////////////////////////////////////////////////////////////
/// Return the singletone instance of the client
///
///////////////////////////////////////////////////////////////////////////////////////
Client& Client::Instance()
{
    return  *m_pClient;
}

////////////////////////////////////////////////////////////////////////////////////////
/// Instantiate client
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::Instantiate(std::string master_address, int master_port, int clientThreadQDepth, std::string scene_name,
                         std::uint32_t width, std::uint32_t height, std::uint32_t rpp)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    if (m_pClient == nullptr)
    {
        m_pClient = new Client(clientThreadQDepth);
        m_pClient->m_master_address = master_address;
        m_pClient->m_master_port = master_port;
        m_pClient->m_scene_name = scene_name;
        m_pClient->m_width = width;
        m_pClient->m_height = height;
        m_pClient->m_rpp = rpp;
        m_pClient->m_SceneWriterMsgQ = std::make_shared<BlockingQueue<MsgQEntry> >(clientThreadQDepth);
        m_pClient->Start();
    }
}

////////////////////////////////////////////////////////////////////////////////////////
/// Client response handler main thread
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::Run()
{
    DEBUG_TRACE("Started Client thread");
    while (1)
    {
        MsgQEntry msgQEntry = TakeNext();
        MsgPtr msgPtr = msgQEntry.m_Msg;
        DEBUG_TRACE("Tid: " << std::hex << std::this_thread::get_id() << ", Client::Run(" << std::hex << this << ") - Received MsgId: "
                    << msgQEntry.m_Msg->GetId() << ", Cmd: " << msgQEntry.m_Cmd.get());
        if (msgQEntry.m_Cmd.get() != nullptr)
        {
            msgQEntry.m_Cmd.get()->ProcessMsg(msgQEntry.m_Msg);
        }
        else
        {
            switch (msgQEntry.m_Msg->GetId())
            {
                case MsgIdServerConstructResponse:
                {
                    OnCreateServerResponse(msgQEntry.m_Msg);
                    break;
                }
                case MsgIdConnectionEstablishmentResponse:
                {
                    OnConnectionEstablishmentResponseMsg(msgQEntry.m_Msg);

                    //store current time to determine total scene completion time
                    start_time = std::chrono::system_clock::now();
                    break;
                }
                case MsgIdSceneProduceRequestAck:
                {
                    OnSceneProduceRequestAckMsg(msgQEntry.m_Msg);
                    break;
                }
                case MsgIdSceneSegmentProduceResponse:
                {
                    OnSceneSegmentProduceRespMsg(msgQEntry.m_Msg);
                    break;
                }

                case MsgIdSceneFileCloseResponse:
                {
                    OnSceneFileCloseResponse(msgQEntry.m_Msg);
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

////////////////////////////////////////////////////////////////////////////////////////
///
/// Unsolicited response handler. All  pixel generation response from workers
/// are handled as unsolicited response to simplify things.
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnTCPRecvMsg(MsgPtr msg)
{
    DEBUG_TRACE("On TCP Recv Message(this:" << std::hex << this << ")");
    std::shared_ptr<TCPRecvMsg> p_recvMsg =  std::dynamic_pointer_cast<TCPRecvMsg>(msg);
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr->GetId())
    {
        case MsgIdSceneSegmentProduceResponse:
        {
            OnSceneSegmentProduceRespMsg(wireMsgPtr);
            break;
        }
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
///
/// Start the client thread
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::Start()
{
    g_pWorkerThread = new WorkerThread("File-writer", m_SceneWriterMsgQ);
    g_pWorkerThread->Start();
    m_thread = new std::thread(&Client::Run, *this);
}

////////////////////////////////////////////////////////////////////////////////////////
///
/// Client server start response handler
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnCreateServerResponse(MsgPtr msg)
{
    DEBUG_TRACE("Worker::OnCreateServerResponse");
    TCPServerConstructStatusMsg *p_responseMsg =  static_cast<TCPServerConstructStatusMsg *>(msg.get());
    m_listening_port  = p_responseMsg->GetPort();

    /// Let's establish a connection
    TransportMgr::Instance().EstablishNewConnection(m_master_address, m_master_port, GetThrdListener(), true);
}


////////////////////////////////////////////////////////////////////////////////////////
///
/// Connection establishment(to master) response handler
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnConnectionEstablishmentResponseMsg(MsgPtr msg)
{
    DEBUG_TRACE("Successfully established connection");
    TCPConnectionEstablishRespMsg *p_responseMsg =  static_cast<TCPConnectionEstablishRespMsg *>(msg.get());
    m_p_ConnectionToMaster = p_responseMsg->GetConnection();
    SceneDescriptorPtr sceneDescriptorPtr = SceneFactory::GetScene(m_scene_name, m_width, m_height, m_rpp);

    /// scene size in pixels.
    m_SceneSizeInPixels = sceneDescriptorPtr->GetNX() * sceneDescriptorPtr->GetNY();

    /// Create a scene writer.
    m_SceneWriterPtr = std::make_shared<SceneWriter>(m_SceneWriterMsgQ, m_scene_name, sceneDescriptorPtr->GetNX(), sceneDescriptorPtr->GetNY());

    /// Create the scene produce request message.
    SceneProduceRequestMsgPtr requestMsg = std::make_shared<SceneProduceRequestMsg>(sceneDescriptorPtr);

    time_t _tm = time(NULL);
    struct tm *curtime = localtime(&_tm);
    std::size_t scene_id = std::hash<std::string>
    {}
    (m_scene_name + asctime(curtime));

    requestMsg->SetSceneId(scene_id);

    DEBUG_TRACE("sceneDescriptorPtr->GetNY():" << sceneDescriptorPtr->GetNY() << "sceneDescriptorPtr->GetNX():" << sceneDescriptorPtr->GetNX());
    requestMsg->SetImageDimension(sceneDescriptorPtr->GetNX(), sceneDescriptorPtr->GetNY(), sceneDescriptorPtr->GetRPP());
    requestMsg->SetAnswerBackAddress(TransportMgr::Instance().MyName(), m_listening_port);
    requestMsg->SetAppTag(m_p_ConnectionToMaster->AllocateAppTag());
    m_p_ConnectionToMaster->SendMsg(requestMsg, GetThrdListener());
}


////////////////////////////////////////////////////////////////////////////////////////
///
/// Scene Produce request acknowledgement request[from master] handler
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnSceneProduceRequestAckMsg(MsgPtr msg)
{
    DEBUG_TRACE("Client::OnSceneProduceRequestAckMsg");
    SceneProduceRequestAckMsgPtr respMsgPtr = std::dynamic_pointer_cast<SceneProduceRequestAckMsg>(msg);
    respMsgPtr->GetConnection()->FreeAppTag(respMsgPtr->GetAppTag());
    if ( respMsgPtr->GetErrorCode() != STATUS_SUCCESS)
    {
       RELEASE_TRACE("Master Rejected the scene produce request(No workers in the cluster!!!!)...");
       exit(0);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
///
/// Scene segment produce response [from client] handler
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnSceneSegmentProduceRespMsg(MsgPtr msg)
{
    SceneSegmentProduceResponseMsgPtr respMsgPtr = std::dynamic_pointer_cast<SceneSegmentProduceResponseMsg>(msg);
    RELEASE_TRACE("Client::Received a scene produce response message, respMsgPtr->GetScenePixelOffset():" << respMsgPtr->GetScenePixelOffset());

    if (m_CurrentPixelToWrite == respMsgPtr->GetScenePixelOffset())
    {
        DEBUG_TRACE("m_CurrentPixelToWrite:" << m_CurrentPixelToWrite << "respMsgPtr->GetScenePixelOffset():" << respMsgPtr->GetScenePixelOffset());

        /// Send file write request
        m_SceneWriterMsgQ->Put(MsgQEntry(respMsgPtr, m_SceneWriterPtr));
        m_CurrentPixelToWrite += respMsgPtr->GetNumPixels();

        /// The pixels are produced in the final format which contains space and end lines. We can remove them and transfer over
        /// the network to reduce network I/O. For time being, let's just stich using the final format.
        for (std::set<MsgPtr>::iterator iter = m_SceneSegmentResponseSet.begin(); iter != m_SceneSegmentResponseSet.end();)
        {
            SceneSegmentProduceResponseMsgPtr segmentMsgPtr = std::dynamic_pointer_cast<SceneSegmentProduceResponseMsg>((*iter));
            if (segmentMsgPtr->GetScenePixelOffset() == m_CurrentPixelToWrite)
            {
                m_SceneWriterMsgQ->Put(MsgQEntry(segmentMsgPtr, m_SceneWriterPtr));
                m_CurrentPixelToWrite += segmentMsgPtr->GetNumPixels();

                iter = m_SceneSegmentResponseSet.erase(iter);
            }
            else
            {
                iter++;
            }
        }

        if (m_CurrentPixelToWrite == m_SceneSizeInPixels)
        {
            /// We are done. close connections and exit.
            m_SceneWriterMsgQ->Put(MsgQEntry(std::make_shared<Msg>(MsgIdSceneFileCloseRequest), m_SceneWriterPtr));
        }
    }
    else
    {
        /// We can get out of order response.. So, we need to handle this carefully. Keep it in sorted
        /// based on the offset and release it as soon as we have enough sequential stuff.
        m_SceneSegmentResponseSet.insert(respMsgPtr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
///
/// Final response handler from scene writer
///
///////////////////////////////////////////////////////////////////////////////////////
void Client::OnSceneFileCloseResponse(MsgPtr msg)
{
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
    RELEASE_TRACE("Successfully produced the image in " + std::to_string(elapsed.count()) + " milliseconds");
    /// NOTE :  We are not cleaning things up interms of shutting down threads and freeing
    /// some dynamically allocated memory explicitly. Exiting from the program will
    /// automatically return the resources to the system.
    exit(0);
}

