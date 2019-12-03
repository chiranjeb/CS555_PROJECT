#include "resource_tracker.hpp"
#include "framework/framework_includes.hpp"
#include "master_scheduler.hpp"

ResourceTracker& ResourceTracker::Instance()
{
    static ResourceTracker s_ResourceTracker;
    return s_ResourceTracker;
}

/// Add entry
void PixelProductionPipelineMgr::AddJob(std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels)
{
    m_OutstandingRequestMap.insert(std::pair<std::string, uint32_t>(std::to_string(sceneId) + ":" + std::to_string(pixelOffset), numPixels));
}


void PixelProductionPipelineMgr::RemoveFailedgJobs(std::size_t sceneId, std::map<uint32_t, uint32_t>& pixelOffsetToCount)
{
    std::map<std::string, uint32_t>::iterator iter = m_OutstandingRequestMap.begin();
    for (; iter != m_OutstandingRequestMap.end();)
    {
        DEBUG_TRACE_APPLICATION("PixelProductionPipelineMgr::RemoveFailedgJobs: " << iter->first);
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
void PixelProductionPipelineMgr::RemoveCompletedJob(std::size_t sceneId, uint32_t pixelOffset)
{
    DEBUG_TRACE_APPLICATION("PixelProductionPipelineMgr::Remove");
    m_OutstandingRequestMap.erase(std::to_string(sceneId) + ":" + std::to_string(pixelOffset));
}

void PixelProductionPipelineMgr::Dump()
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
        uint32_t totalNumPixelsTobeScheduled;
        uint32_t minPixelChunkSize = SchedulingPolicyParam::Get().m_DynamicSchedulePixelChunkMin;
        do
        {
            totalNumPixelsTobeScheduled = minPixelChunkSize * m_TotalNumPixelProductionPipelines;
            minPixelChunkSize *= 2;
        }
        while ((totalNumOfPixels > (minPixelChunkSize * 2 * m_TotalNumPixelProductionPipelines)) && (minPixelChunkSize <= SchedulingPolicyParam::Get().m_DynamicScheduleInitialPixelChunkMax));

        DEBUG_TRACE_APPLICATION("\t" << "ResourceTracker:GetWorkEstimationForNewScene (initial pixels per pipeline): " << std::min(totalNumPixelsTobeScheduled, totalNumOfPixels)/m_TotalNumPixelProductionPipelines);
        return std::min(totalNumPixelsTobeScheduled, totalNumOfPixels) ; 
    }
}

/// Add a new worker. Create context for all hw execution pipelines.
void ResourceTracker::AddWorker(std::string host_name, uint16_t numAvailableHwExecutionThread, uint32_t PixelProductionTimeInSecForKnownScene)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    ResourceEntryPtr resourceEntryPtr = std::make_shared<ResourceEntry>(host_name, numAvailableHwExecutionThread, PixelProductionTimeInSecForKnownScene);
    m_HostWorkers.push_back(resourceEntryPtr);

    m_WorkerLookupTable.insert(std::pair<std::string, ResourceEntryPtr>(host_name, resourceEntryPtr));

    if (PixelProductionTimeInSecForKnownScene < m_MaxPixelProductionRate)
    {
        m_MaxPixelProductionRate = PixelProductionTimeInSecForKnownScene;
    }

    /// Add total number of hardware pipelines.
    m_TotalNumPixelProductionPipelines += numAvailableHwExecutionThread;

    /// Generate h/w pipeline managers who will keep track of the works.
    for (uint16_t index = 0; index < numAvailableHwExecutionThread; ++index)
    {
        PixelProductionPipelineMgr *p_pipeline_mgr = new PixelProductionPipelineMgr(host_name, index);
        m_WorkerToPixelProductionPipelineMgr.insert(std::pair<std::string, PixelProductionPipelineMgr *>(host_name + ":" + std::to_string(index), p_pipeline_mgr));
    }

    /// Dump the worker lists
    DEBUG_TRACE_APPLICATION("worker list: " << m_HostWorkers.size());
    for (std::vector<ResourceEntryPtr>::iterator iter = m_HostWorkers.begin(); iter != m_HostWorkers.end(); iter++)
    {
        DEBUG_TRACE_APPLICATION("worker: " << (*iter)->m_UniqueHostName << ", num logical pipelines:"
                                << (*iter)->m_NumAvailablePixelProductionPipelines << ", m_PixelProductionTimeInSecForKnownScene: "
                                << (*iter)->m_PixelProductionTimeInSecForKnownScene);
    }



    DEBUG_TRACE_APPLICATION(" ResourceTracker::AddWorker max pixel production rate" << m_MaxPixelProductionRate);

    Dump();

}

void ResourceTracker::NotifyHostFailure(std::string hostname)
{
    DEBUG_TRACE_APPLICATION(" ResourceTracker::NotifyHostFailure");
    std::unique_lock<std::mutex> lck(m_Mutex);
    uint32_t num_pipelines = 0;
    auto iter = m_HostWorkers.begin();
    for (; iter != m_HostWorkers.end(); ++iter)
    {
        if (iter->get()->m_UniqueHostName.compare(hostname) == 0)
        {
            num_pipelines = iter->get()->m_NumAvailablePixelProductionPipelines;
            break;
        }
    }

    DEBUG_TRACE_APPLICATION(" ResourceTracker::NotifyHostFailure, num_pipelines being removed" << num_pipelines);
    if (iter != m_HostWorkers.end())
    {
        m_TotalNumPixelProductionPipelines -=  num_pipelines;
    }
}


void ResourceTracker::TrackJob(std::string hostname, uint16_t pixelProductionPipelineId, std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    PixelProductionPipelineMgr *pPixelProductionPipelineMgr = m_WorkerToPixelProductionPipelineMgr[hostname +  ":" + std::to_string(pixelProductionPipelineId)];
    pPixelProductionPipelineMgr->AddJob(sceneId, pixelOffset,  numPixels);
}

void ResourceTracker::RemoveFailedJobs(std::string hostname, std::size_t sceneId, std::map<uint32_t, uint32_t>& outstandingJobs)
{
    DEBUG_TRACE_APPLICATION(" ResourceTracker::RemoveFailedJobs");
    std::unique_lock<std::mutex> lck(m_Mutex);
    uint32_t num_pipelines = 0;
    auto iter = m_HostWorkers.begin();
    for (; iter != m_HostWorkers.end(); ++iter)
    {
        if (iter->get()->m_UniqueHostName.compare(hostname) == 0)
        {
            num_pipelines = iter->get()->m_NumAvailablePixelProductionPipelines;
            DEBUG_TRACE_APPLICATION(" ResourceTracker::num_pipelines" << num_pipelines);
            break;
        }
    }

    if (iter != m_HostWorkers.end())
    {
        uint32_t totalNumEntries = 0, totalNumEntriesRemoved = 0;
        for (int pixelProductionPipelineId = 0; pixelProductionPipelineId < num_pipelines; ++pixelProductionPipelineId)
        {
            if (m_WorkerToPixelProductionPipelineMgr.find(hostname +  ":" + std::to_string(pixelProductionPipelineId)) != m_WorkerToPixelProductionPipelineMgr.end())
            {
                DEBUG_TRACE_APPLICATION(" ResourceTracker::RemoveFailedJobs, hostname: " << hostname << "pixelProductionPipelineId" << pixelProductionPipelineId);
                totalNumEntries++;
                PixelProductionPipelineMgr *pPixelProductionPipelineMgr = m_WorkerToPixelProductionPipelineMgr[hostname +  ":" + std::to_string(pixelProductionPipelineId)];
                pPixelProductionPipelineMgr->RemoveFailedgJobs(sceneId, outstandingJobs);
                if (pPixelProductionPipelineMgr->m_OutstandingRequestMap.empty())
                {
                    totalNumEntriesRemoved++;
                    /// This h/w pipeline manager is empty. Remove it
                    m_WorkerToPixelProductionPipelineMgr.erase(hostname +  ":" + std::to_string(pixelProductionPipelineId));
                }
            }
        }

        if (totalNumEntries == 0 || (totalNumEntries ==  totalNumEntriesRemoved))
        {
            m_HostWorkers.erase(iter);
            m_WorkerLookupTable.erase(iter->get()->m_UniqueHostName);
        }
    }

    DEBUG_TRACE_APPLICATION("!!!!!!ResourceTracker::RemoveFailedJobs Done!!!!!!!");
    Dump();
}


void ResourceTracker::NotifyJobDone(std::string hostname, uint16_t pixelProductionPipelineId, std::size_t sceneId, uint32_t pixelOffset)
{
    std::unique_lock<std::mutex> lck(m_Mutex);
    DEBUG_TRACE_APPLICATION("ResourceTracker::NotifyJobDone" << ", pixelProductionPipelineId:" << pixelProductionPipelineId << ", sceneId:" << sceneId << ", pixelOffset:" << pixelOffset);
    PixelProductionPipelineMgr *pPixelProductionPipelineMgr = m_WorkerToPixelProductionPipelineMgr[hostname + ":" + std::to_string(pixelProductionPipelineId)];
    pPixelProductionPipelineMgr->RemoveCompletedJob(sceneId, pixelOffset);
}




void ResourceTracker::Dump()
{
    DEBUG_TRACE_APPLICATION("ResourceTracker::Dump ");
    auto iter = m_WorkerToPixelProductionPipelineMgr.begin();
    for (; iter != m_WorkerToPixelProductionPipelineMgr.end(); ++iter)
    {
        DEBUG_TRACE_APPLICATION("Hw Thread Name: " << iter->first);
        iter->second->Dump();
    }
}

