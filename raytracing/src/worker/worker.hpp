#pragma once

#include "framework/framework_includes.hpp"
#include "defines/defines_includes.hpp"

class TCPIOConnection;
class Worker : public MsgQThread
{
    static const int WORKER_CMD_PROCESSOR_MSG_Q_DEPTH = 128;
public:
    Worker() : MsgQThread("Worker", WORKER_CMD_PROCESSOR_MSG_Q_DEPTH)
    {
    }

    /// Get the Worker
    static Worker &Instance()
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

protected:

    /// Run the worker thread
    void Run();

    /// Connection establishment response msg
    void OnCreateServerResponse(MsgPtr msg);

    /// Connection establishment response msg
    void OnConnectionEstablishmentResponseMsg(MsgPtr msg);

    /// Worker registration response msg
    void OnWorkerRegistrationRespMsg(MsgPtr msg);

    TCPIOConnection *m_p_ConnectionToMaster;
    std::string m_master_address;
    int m_master_port;
    int m_listening_port;
};
