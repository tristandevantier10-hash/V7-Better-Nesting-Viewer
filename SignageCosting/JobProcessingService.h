#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "Job.h"
#include "CostEngine.h"
#include "PendingResults.h"

class JobProcessingService
{
public:
    JobProcessingService() = default;
    ~JobProcessingService();

    JobProcessingService(const JobProcessingService&) = delete;
    JobProcessingService& operator=(const JobProcessingService&) = delete;

    void start();
    void stop();
    bool submit(const Job& job);
    bool tryTakeResult(PendingResults& result);
    void clearPendingResults();

private:
    void run();

    CostEngine engine;
    std::queue<Job> jobs;
    std::mutex jobsMutex;
    std::condition_variable jobsAvailable;
    bool running = false;
    std::thread worker;

    PendingResults pendingResults;
    std::mutex resultMutex;
};
