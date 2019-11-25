#pragma once
#include<iostream>
#include<string>
#include<memory>
#include<mutex>
#include<vector>
#include<map>

struct Pixel2XYMapper
{
    Pixel2XYMapper(int Ny, int Nx, int pixelPos)
    {
        Y = Ny - ((pixelPos / Nx) + 1);
        X = pixelPos % Nx;
    }
    uint16_t X;
    uint16_t Y;
};


/// Resource entry
struct ResourceEntry
{
    ResourceEntry(std::string host_name, uint16_t numAvailableHwExecutionThread) :
        m_UniqueHostName(host_name), m_NumAvailableHwExecutionThread(numAvailableHwExecutionThread)
    {
    }

    std::string m_UniqueHostName;
    int m_NumAvailableHwExecutionThread;
};

struct HwThreadMgr
{
    /// Constructor
    HwThreadMgr(std::string hostname, uint32_t threadId) :
        m_UniqueHostName(hostname), m_ThreadId(threadId)
    {
    }

    /// Add a new job
    void AddJob(std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels); 

    /// Remove a job
    void RemoveJob(std::size_t sceneId, uint32_t pixelOffset);

    void Dump();


    std::string m_UniqueHostName;   ///< hostname
    uint32_t m_ThreadId;              ///< thread id
                                      /// Add capability param to deal CPU cores with different speed.
    /// <Scene Id, start offset>  => Pixel ProduceRequestMsgPtr
    std::map<std::string, uint32_t> m_OutstandingRequestMap;
};




typedef std::shared_ptr<ResourceEntry> ResourceEntryPtr;

class ResourceTracker
{
public:
    /// constructor
    ResourceTracker()
    {
        m_total_num_hw_threads = 0;
    }

    /// Get resoource tracker instance
    static ResourceTracker& Instance();

    /// Get Total number of available h/w threads.
    uint16_t GetTotalNumberOfHwThreads()
    {
        return m_total_num_hw_threads;
    }

    /// Return the workers
    std::vector<ResourceEntryPtr>& GetHostWorkers()
    {
        return m_HostWorkers;
    }


    /// Get work estimation for new scene. Returns amount of pixels which could be 
    /// assigned to all the threads.
    uint32_t GetWorkEstimationForNewScene(uint32_t totalNumOfPixels);
    
    /// Get work estimation for a thread which has just completed some work.
    void GetWorkEstimation(std::string host_name, uint16_t threadId, uint32_t pendingPixels);

    /// Add a new worker to the system
    void AddWorker(std::string host_name, uint16_t numAvailableHwExecutionThread);

    /// Track job
    void TrackJob(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset, uint32_t numPixels);

    /// Notify job done
    void NotifyJobDone(std::string hostname, uint16_t thread_id, std::size_t sceneId, uint32_t pixelOffset);

    void Dump();

protected:
    uint32_t m_total_num_hw_threads;
    std::mutex m_Mutex;   /// We can be much more smarter of not using this lock too much. 
    std::vector<ResourceEntryPtr> m_HostWorkers;

    /// <worker-name, thread Id> <=> HwThreadContext*
    std::map<std::string, HwThreadMgr *> m_WorkerThreads;
};
