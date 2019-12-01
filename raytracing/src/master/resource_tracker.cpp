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


void HwThreadMgr::RemoveFailedgJobs(std::size_t sceneId, std::map<uint32_t, uint32_t>& pixelOffsetToCount)
{
    std::map<std::string, uint32_t>::iterator iter = m_OutstandingRequestMap.begin();
    for (; iter != m_OutstandingRequestMap.end();)
    {
        DEBUG_TRACE_APPLICATION("HwThreadMgr::RemoveFailedgJobs: " << iter->first);
        std::stringstream keystream(iter->first);
        std::string scene, pixelOffsetS;
        std::getline(keystream, scene, ':');
        std::getline(keystream, pixelOffsetS, ':');

        if (std::to_string(sceneId).compare(scene) == 0)
        {
            uint32_t pixelOffset;
            std::istringstream pixelOffsetStream(pixelOffsetS);
            pixelOffsetStream >> pixelOffset;
            pixelOffsetToCount.insert(std::pair<uint32_t, uint32_t>(pixelOffset, iter->second));
            iter = m_OutstandingRequestMap.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}


/// Remove entry
void HwThreadMgr::RemoveCompletedJob(std::size_t sceneId, uint32_t pixelOffset)
{
    DEBUG_TRACE_APPLICATION("HwThreadMgr::Remove");
    m_OutstandingRequestMap.erase(std::to_string(sceneId) + ":" + std::to_string(pixelOffset));
}

void HwThreadMgr::Dump()
{
    for (auto iter = m_OutstandingRequestMap.begin(); iter != m_OutstandingRequestMap.end(); ++iter)
    {
        DEBUG_TRACE_APPLICATION("\t" << "scene:pixeloffset: " << iter->first << ", numPixels: " << iter->second)
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
    DEBUG_TRACE_APPLICATION("worker list: " << m_HostWorkers.size());
    for (std::vector<ResourceEntryPtr>::iterator iter = m_HostWorkers.begin(); iter != m_HostWorkers.end(); iter++)
    {
        DEBUG_TRACE_APPLICATION("worker: " << (*iter)->m_UniqueHostName);
    }

}

void ResourceTracker::NotifyHostFailure(std::string hostname)
{
    DEBUG_TRACE_APPLICATION(" ResourceTracker::NotifyHostFailure");
    std::unique_lock<std::mutex> lck(m_Mutex);
    uint32_t num_threads = 0;
    auto iter = m_HostWorkers.begin();
    for (; iter != m_HostWorkers.end(); ++iter)
    {
        if (iter->get()->m_UniqueHostName.compare(hostname) == 0)
        {
            num_threads = iter->get()->m_NumAvailableHwExecutionThread;
            break;
        }
    }

    DEBUG_TRACE_APPLICATION(" ResourceTracker::NotifyHostFailure, num_threads being removed" << num_threads);
    if (iter != m_HostWorkers.end())
    {
        m_total_num_hw_threads -=  num_threads;
    }
}


void ResourceTracker::TrackJob(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    HwThreadMgr *pHwThreadMgr = m_WorkerThreads[hostname +  ":" + std::to_string(thread_id)];
    pHwThreadMgr->AddJob(sceneId, pixelOffset,  numPixels);
}

void ResourceTracker::RemoveFailedJobs(std::string hostname, std::size_t sceneId, std::map<uint32_t, uint32_t>& outstandingJobs)
{
    DEBUG_TRACE_APPLICATION(" ResourceTracker::RemoveFailedJobs");
    std::unique_lock<std::mutex> lck(m_Mutex);
    uint32_t num_threads = 0;
    auto iter = m_HostWorkers.begin();
    for (; iter != m_HostWorkers.end(); ++iter)
    {
        if (iter->get()->m_UniqueHostName.compare(hostname) == 0)
        {
            num_threads = iter->get()->m_NumAvailableHwExecutionThread;
            DEBUG_TRACE_APPLICATION(" ResourceTracker::num_threads" << num_threads);
            break;
        }
    }

    if (iter != m_HostWorkers.end())
    {
        uint32_t totalNumEntries = 0, totalNumEntriesRemoved = 0;
        for (int thread_id = 0; thread_id < num_threads; ++thread_id)
        {
            if (m_WorkerThreads.find(hostname +  ":" + std::to_string(thread_id)) != m_WorkerThreads.end())
            {
                DEBUG_TRACE_APPLICATION(" ResourceTracker::RemoveFailedJobs, hostname: " << hostname << "thread_id" << thread_id);
                totalNumEntries++;
                HwThreadMgr *pHwThreadMgr = m_WorkerThreads[hostname +  ":" + std::to_string(thread_id)];
                pHwThreadMgr->RemoveFailedgJobs(sceneId, outstandingJobs);
                if (pHwThreadMgr->m_OutstandingRequestMap.empty())
                {
                    totalNumEntriesRemoved++;
                    /// This h/w thread manager is empty. Remove it
                    m_WorkerThreads.erase(hostname +  ":" + std::to_string(thread_id));
                }
            }
        }

        if (totalNumEntries == 0 || (totalNumEntries ==  totalNumEntriesRemoved))
        {
            m_HostWorkers.erase(iter);
        }
    }

    DEBUG_TRACE_APPLICATION("!!!!!!ResourceTracker::RemoveFailedJobs Done!!!!!!!");
    Dump();
}


void ResourceTracker::NotifyJobDone(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    DEBUG_TRACE_APPLICATION("ResourceTracker::NotifyJobDone" << ", thread_id:" << thread_id << ", sceneId:" << sceneId << ", pixelOffset:" << pixelOffset);
    HwThreadMgr *pHwThreadMgr = m_WorkerThreads[hostname + ":" + std::to_string(thread_id)];
    pHwThreadMgr->RemoveCompletedJob(sceneId, pixelOffset);
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

