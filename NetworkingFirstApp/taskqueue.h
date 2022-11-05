#pragma once
#include <algorithm>
#include <deque>
#include <future>
#include <mutex>

class TaskQueue
{
public:

    explicit TaskQueue(unsigned numWorkers = 0);

    ~TaskQueue();

    void abort();

    /// <summary> Stops new work from being submitted to this work queue.</summary>
    void stop();

    /// <summary> Wait for completion of all submitted work. No more work will 
    /// be allowed to be submitted. </summary>
    void waitForCompletion();


    template<typename RETVAL>
    std::future<RETVAL> submit(std::function<RETVAL()> function);

    template<>
    std::future<void> submit(std::function<void()> function);

    void operator=(const TaskQueue&) = delete;
    TaskQueue(const TaskQueue&) = delete;

private:
    std::deque<std::function<void()>> m_queueOfTasks;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
    std::atomic<bool> m_exit{ false };
    std::atomic<bool> m_completeQueuedTasks{ true };
    std::vector<std::thread> m_workingThreads;

    void doWork();
    void joinAll();
};

template <typename RETVAL>
std::future<RETVAL> TaskQueue::submit(std::function<RETVAL()> function)
{
    if (m_exit) 
    {
        throw std::runtime_error("Caught work submission to work queue that is desisting.");
    }

    using promiseFunctionPair = std::pair<std::promise<RETVAL>, std::function<RETVAL()>>;
    auto data = std::make_shared<promiseFunctionPair>(std::promise<RETVAL>(), std::move(function));

    std::future<RETVAL> future = data->first.get_future();

    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_queueOfTasks.emplace_back([data]() 
        {
            try 
            {
                data->first.set_value(data->second());
            }
            catch (...) 
            {
                data->first.set_exception(std::current_exception());
            }
        });
    }
    m_condVar.notify_one();
    return std::move(future);
}


