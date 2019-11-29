#pragma once
#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////
///
/// This command writes the scene file. Goal is to free the client response handler thread
/// while the file write is in progress. 
///
///////////////////////////////////////////////////////////////////////////////////////
class SceneWriter : public Command
{
public:
    /// Scene writer constructor.
    SceneWriter(BlockingMsgQPtr q, std::string scene_name, int X, int Y);

    /// Scene writer message handler
    virtual void ProcessMsg(MsgPtr msg);

protected:
    /// Scene file produce response message.
    void OnSegmentProduceResonse(MsgPtr msg);

    /// Scene file close request.
    void OnSceneFileCloseRequest(MsgPtr msg);

    /// scene file stream pointer
    std::ofstream m_file_output_stream;

    /// offset to the file for debug print purpose only.
    int m_offset;
};



