#include "taskqueue.h"
#include <cassert>
#include <iostream>

TaskQueue::TaskQueue(unsigned numWorkers)
{
    if (numWorkers < 1) 
    {
        numWorkers = std::thread::hardware_concurrency() - 1;
    }
    while (numWorkers--) 
    {
        m_workingThreads.emplace_back(std::thread(&TaskQueue::doWork, this));
    }
}

TaskQueue::~TaskQueue()
{
    abort();
}

void TaskQueue::abort()
{
    m_exit = true;
    m_completeQueuedTasks = false; // prohibit completing of queued tasks
    m_condVar.notify_all();
    joinAll();

    {
        std::lock_guard lg(m_mutex);
        m_queueOfTasks.clear();
    }
}

void TaskQueue::stop()
{
    m_exit = true;
    m_completeQueuedTasks = true; // allow completing of queued tasks
    m_condVar.notify_all();
}

void TaskQueue::waitForCompletion()
{
    stop();
    joinAll();
    assert(m_queueOfTasks.empty());
}

template <>
std::future<void> TaskQueue::submit<void>(std::function<void()> function)
{
    if (m_exit) 
    {
        throw std::runtime_error("Caught work on exit.");
    }

    using promiseFunctionPair = std::pair<std::promise<void>, std::function<void()>>;
    auto data = std::make_shared<promiseFunctionPair>(std::promise<void>(), std::move(function));

    std::future<void> future = data->first.get_future();

    {
        std::lock_guard lg(m_mutex);
        m_queueOfTasks.emplace_back([data]() 
        {
            auto&& [promise, lambda] = *data;
            try 
            {
                lambda();
                promise.set_value();
            }
            catch (...) 
            {
                promise.set_exception(std::current_exception());
            }
        });
    }
    m_condVar.notify_one();
    return future;
}

void TaskQueue::doWork()
{
    const bool isNotEmpty = !m_queueOfTasks.empty();
    while (!m_exit || (m_completeQueuedTasks && isNotEmpty))
    {
        std::unique_lock ul(m_mutex);
        std::cout << "Thread " << std::this_thread::get_id() << " is ready" << std::endl;
        m_condVar.wait(ul, [this]() {return !m_queueOfTasks.empty(); });
        std::function work(std::move(m_queueOfTasks.front()));
        m_queueOfTasks.pop_front();
        std::cout << "Thread " << std::this_thread::get_id() << " got a new task" << std::endl;
        ul.unlock();
        work();
    }
}

void TaskQueue::joinAll()
{
    for (auto& thread : m_workingThreads) 
    {
        thread.join();
    }
    m_workingThreads.clear();
}
