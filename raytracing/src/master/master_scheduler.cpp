#include <iostream>
#include "master_scheduler.hpp"
#include "transport/transport_mgr.hpp"
#include "transport/transport_msgs.hpp"
#include "transport/tcp_io_connection.hpp"
#include "wiremsg/worker_registration_msg.hpp"
#include "wiremsg/scene_description_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"

void MasterScheduler::Run()
{
    while (1)
    {
        MsgQEntry msgQEntry = TakeNext();
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
                    DEBUG_TRACE("Server Started");
                    break;

                case MsgIdTCPRecv:
                    OnTCPRecvMsg(msgQEntry.m_Msg);
                    break;

                default:
                    break;
            }
        }
    }
}

void MasterScheduler::OnTCPRecvMsg(MsgPtr msg)
{
    DEBUG_TRACE("On TCP Recv Message");
    TCPRecvMsg *p_recvMsg =  static_cast<TCPRecvMsg *>(msg.get());
    WireMsgPtr wireMsgPtr = p_recvMsg->GetWireMsg();

    switch (wireMsgPtr.get()->GetId())
    {
        case MsgIdWorkerRegistrationRequest:
        {
            OnWorkerRegistrationRequest(wireMsgPtr);
            break;
        }
        default:
            break;
    }
}

void MasterScheduler::OnWorkerRegistrationRequest(WireMsgPtr wireMsgPtr)
{
    WorkerRegistrationMsg *pWireMsg = static_cast<WorkerRegistrationMsg *>(wireMsgPtr.get());
    pWireMsg->Dump();

    /// Register the host name
    std::string unique_hostname = UniqueServerId(pWireMsg->m_hostname, pWireMsg->m_Port).toString();
    m_workerlist.push_back(unique_hostname);
    TransportMgr::Instance().SaveConnection(unique_hostname, pWireMsg->GetConnection());
    WorkerRegistrationRespMsgPtr reigstrationRespMsgPtr =  WorkerRegistrationRespMsgPtr(new WorkerRegistrationRespMsg(STATUS_SUCCESS));
    reigstrationRespMsgPtr.get()->SetAppTag(pWireMsg->GetAppTag());

    /// We are not expecting any response back. So, pass a nullptr.
    pWireMsg->GetConnection()->SendMsg(reigstrationRespMsgPtr, nullptr);

    /// Dump the worker lists
    DEBUG_TRACE("worker list: " << m_workerlist.size());
    for (std::vector<std::string>::iterator iter = m_workerlist.begin(); iter != m_workerlist.end(); iter++)
    {
        DEBUG_TRACE("worker: " << *iter);
    }

    // @todo: This should come from a client ideally.
    // For now, just get the scene descriptor and send to all the clients.
    /// Send the scene file.
    //SceneDescriptorPtr sceneDescriptorPtr = SceneFactory::GenerateRandomScene();
    //SceneDescriptionMsgPtr sceneDescriptorMsg = std::make_shared<SceneDescriptionMsg>(sceneDescriptorPtr);
    //TransportMgr::Instance().FindConnection(m_workerlist[0])->SendMsg(sceneDescriptorMsg, GetThrdListener());
}



void MasterScheduler::OnNewSceneGeneration(WireMsgPtr wireMsgPtr)
{
#if 0
    // For each worker, distribute the load. Find out the equally distributed workload. Identify partition count.
    int numPartition = 1; // Let's check the network serialization/deserialization.
    for (int index = 0; index < numPartition; ++index)
    {
        // Create a scene description message for each  worker. Include some work in the scene description too.
        SceneDescriptionMsgPtr sceneDescriptorMsg = std::make_shared<SceneDescriptionMsg>(sceneDescriptorPtr);
        // Note we are not waiting for the response from the worker. However, we want to make sure the message
        // has been sent out by the transport layer otherwise  we need to do fault handling.
        TransportMgr::Instance().FindConnection(m_workerlist[index])->SendMsg(sceneDescriptorMsg, GetThrdListener());
    }
#endif

}


