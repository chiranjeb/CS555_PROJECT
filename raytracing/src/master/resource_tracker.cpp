#include "resource_tracker.hpp"
#include "framework/framework_includes.hpp"
#include "master_scheduler.hpp"

ResourceTracker& ResourceTracker::Instance()
{
    static ResourceTracker s_ResourceTracker;
    return s_ResourceTracker;
}

/// Add entry
void HwThreadMgr::AddJob(std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels)
{
    m_OutstandingRequestMap.insert(std::pair<std::string, uint32_t>(std::to_string(sceneId) + ":" + std::to_string(pixelOffset), numPixels));
}

/// Remove entry
void HwThreadMgr::RemoveJob(std::size_t sceneId, uint32_t pixelOffset)
{
    DEBUG_TRACE_APPLICATION("HwThreadMgr::Remove");
    m_OutstandingRequestMap.erase(std::to_string(sceneId) + ":" + std::to_string(pixelOffset));
}

void HwThreadMgr::Dump()
{
    for (auto iter = m_OutstandingRequestMap.begin(); iter != m_OutstandingRequestMap.end(); ++iter)
    {
        DEBUG_TRACE("\t" << "scene:pixeloffset: " << iter->first << ", numPixels: " << iter->second)
    }
}

uint32_t ResourceTracker::GetWorkEstimationForNewScene(uint32_t totalNumOfPixels)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    if (SchedulingPolicyParam::Get().ConfiguredAsStaticScheduler())
    {
        /// Static scheduling is non-interleaved and all the pixels are scheduled all at once.
        return totalNumOfPixels;
    }
    else
    {
        /// todo : for dynamic schedule, we can evaluate based on the current load about how much to schedule.
        /// Let's give a flat 4096 number of pixels to begin with and then we will adjust.
        return 2048 * m_total_num_hw_threads;
    }
}

/// Add a new worker. Create context for all hw execution threads.
void ResourceTracker::AddWorker(std::string host_name, uint16_t numAvailableHwExecutionThread)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    m_HostWorkers.push_back(std::make_shared<ResourceEntry>(host_name, numAvailableHwExecutionThread));

    /// Add total number of hardware threads.
    m_total_num_hw_threads += numAvailableHwExecutionThread;

    /// Generate h/w thread managers who will keep track of the works.
    for (uint16_t index = 0; index < numAvailableHwExecutionThread; ++index)
    {
        HwThreadMgr *p_thread_mgr = new HwThreadMgr(host_name, index);
        m_WorkerThreads.insert(std::pair<std::string, HwThreadMgr *>(host_name + ":" + std::to_string(index), p_thread_mgr));
    }

    /// Dump the worker lists
    DEBUG_TRACE("worker list: " << m_HostWorkers.size());
    for (std::vector<ResourceEntryPtr>::iterator iter = m_HostWorkers.begin(); iter != m_HostWorkers.end(); iter++)
    {
        DEBUG_TRACE("worker: " << (*iter)->m_UniqueHostName);
    }

}


void ResourceTracker::TrackJob(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    HwThreadMgr *pHwThreadMgr = m_WorkerThreads[hostname +  ":" + std::to_string(thread_id)];
    pHwThreadMgr->AddJob(sceneId, pixelOffset,  numPixels);
}


void ResourceTracker::NotifyJobDone(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    DEBUG_TRACE_APPLICATION("ResourceTracker::NotifyJobDone" << ", thread_id:" << thread_id << ", sceneId:" <<sceneId << ", pixelOffset:" << pixelOffset);
    HwThreadMgr *pHwThreadMgr = m_WorkerThreads[hostname + ":" + std::to_string(thread_id)];
    pHwThreadMgr->RemoveJob(sceneId, pixelOffset);
}




void ResourceTracker::Dump()
{
    DEBUG_TRACE("ResourceTracker::Dump ");
    auto iter = m_WorkerThreads.begin();
    for (; iter != m_WorkerThreads.end(); ++iter)
    {
        DEBUG_TRACE("Hw Thread Name: " << iter->first);
        iter->second->Dump();
    }
}

