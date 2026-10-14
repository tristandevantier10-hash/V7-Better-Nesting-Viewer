#include "JobProcessingService.h"

#include <utility>

#include "InvoicePrinter.h"
#include "ResultsViewModelBuilder.h"

JobProcessingService::~JobProcessingService()
{
    stop();
}

void JobProcessingService::start()
{
    {
        std::lock_guard<std::mutex> lock(jobsMutex);
        if (running || worker.joinable())
            return;

        running = true;
    }

    worker = std::thread(&JobProcessingService::run, this);
}

void JobProcessingService::stop()
{
    {
        std::lock_guard<std::mutex> lock(jobsMutex);
        running = false;
    }

    jobsAvailable.notify_one();

    if (worker.joinable())
        worker.join();
}

bool JobProcessingService::submit(const Job& job)
{
    {
        std::lock_guard<std::mutex> lock(jobsMutex);
        if (!running)
            return false;

        jobs.push(job);
    }

    jobsAvailable.notify_one();
    return true;
}

bool JobProcessingService::tryTakeResult(PendingResults& result)
{
    std::lock_guard<std::mutex> lock(resultMutex);
    if (!pendingResults.ready)
        return false;

    result = std::move(pendingResults);
    pendingResults = PendingResults();
    return true;
}

void JobProcessingService::clearPendingResults()
{
    std::lock_guard<std::mutex> lock(resultMutex);
    pendingResults = PendingResults();
}

void JobProcessingService::run()
{

    while (true)
    {
        Job job;
        {
            std::unique_lock<std::mutex> lock(jobsMutex);
            jobsAvailable.wait(lock, [this]()
            {
                return !running || !jobs.empty();
            });

            if (!running)
                break;

            job = std::move(jobs.front());
            jobs.pop();
        }

        CostResult result = engine.calculate(job);
        ResultsViewModel viewModel = ResultsViewModelBuilder::build(result);

        {
            std::lock_guard<std::mutex> lock(resultMutex);
            pendingResults.result = result;
            pendingResults.viewModel = viewModel;
            pendingResults.ready = true;
        }

        InvoicePrinter::print(result);
    }

}
