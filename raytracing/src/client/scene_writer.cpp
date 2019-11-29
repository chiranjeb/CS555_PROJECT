#include "scene_writer.hpp"
#include "wiremsg/scene_produce_msg.hpp"
#include "client.hpp"

////////////////////////////////////////////////////////////////////////////////////////
///
/// Constructor
///
///////////////////////////////////////////////////////////////////////////////////////
SceneWriter::SceneWriter(BlockingMsgQPtr q, std::string scene_name, int X, int Y) : Command(q)
{
    /// Create the output file pointer.
    m_file_output_stream.open(scene_name + ".ppm");

    /// write header
    m_file_output_stream << "P3\n" << X << " " << Y << "\n255\n";

    m_offset = 0;
}


////////////////////////////////////////////////////////////////////////////////////////
///
/// Command Message handler
///
///////////////////////////////////////////////////////////////////////////////////////
void SceneWriter::ProcessMsg(MsgPtr msg)
{
    DEBUG_TRACE("Tid: " << std::hex << std::this_thread::get_id() << ", SceneWriter::ProcessMsg("
                << std::hex << this << ") - Received MsgId: " << msg->GetId());

    switch (msg->GetId())
    {
       case MsgIdSceneSegmentProduceResponse:
           OnSegmentProduceResonse(msg);
           break;
       case MsgIdSceneFileCloseRequest:
           OnSceneFileCloseRequest(msg);
           break;
       default:
           break;
    }
}


////////////////////////////////////////////////////////////////////////////////////////
///
/// Scene Segment produce response handler
///
///////////////////////////////////////////////////////////////////////////////////////
void SceneWriter::OnSegmentProduceResonse(MsgPtr msg)
{
    SceneSegmentProduceResponseMsgPtr respMsgPtr = std::dynamic_pointer_cast<SceneSegmentProduceResponseMsg>(msg);
    std::pair<uint8_t *, uint32_t> sceneSegmentBuffer = respMsgPtr->GetSceneBuffer();
    RELEASE_TRACE("Writing scene file. offset: " << m_offset << ", size: " << respMsgPtr->GetNumPixels());
    m_file_output_stream.write(reinterpret_cast<char *>(sceneSegmentBuffer.first), sceneSegmentBuffer.second);
    m_offset += respMsgPtr->GetNumPixels();
}

////////////////////////////////////////////////////////////////////////////////////////
///
/// Scene file close request handler
///
///////////////////////////////////////////////////////////////////////////////////////
void SceneWriter::OnSceneFileCloseRequest(MsgPtr msg)
{
    m_file_output_stream.close();
    Client::Instance().GetThrdListener()->Notify(std::make_shared<Msg>(MsgIdSceneFileCloseResponse));
}

