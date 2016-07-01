#include "pool.h"
#include <cassert>
#include <algorithm>

namespace thread
{
        static std::size_t n_active_workers(const std::vector<worker_t>& workers)
        {
                return  static_cast<std::size_t>(std::count_if(workers.begin(), workers.end(),
                        [] (const auto& worker) { return worker.active(); }));
        }
}

thread::pool_t& thread::pool_t::instance()
{
        static pool_t the_pool;
        return the_pool;
}

thread::pool_t::pool_t()
{
        const auto n_workers = static_cast<std::size_t>(nano::logical_cpus());
        const auto n_active_threads = n_workers;

        for (size_t i = 0; i < n_workers; ++ i)
        {
                m_workers.emplace_back(m_queue, i < n_active_threads);
        }
        for (size_t i = 0; i < n_workers; ++ i)
        {
                m_threads.emplace_back(std::ref(m_workers[i]));
        }

        assert(n_active_threads == thread::n_active_workers(m_workers));
}

thread::pool_t::~pool_t()
{
        // stop & join
        {
                const std::lock_guard<std::mutex> lock(m_queue.m_mutex);

                m_queue.m_stop = true;
                m_queue.m_condition.notify_all();
        }

        for (auto& thread : m_threads)
        {
                thread.join();
        }
}

void thread::pool_t::activate(std::size_t count)
{
        const std::lock_guard<std::mutex> lock(m_queue.m_mutex);

        count = std::max(std::size_t(1), std::min(count, n_workers()));

        std::size_t crt_count = thread::n_active_workers(m_workers);
        assert(crt_count > 0);
        for (auto& worker :  m_workers)
        {
                if (crt_count == count)
                {
                        break;
                }

                else if (crt_count > count)
                {
                        if (worker.deactivate())
                        {
                                -- crt_count;
                        }
                }

                else if (crt_count < count)
                {
                        if (worker.activate())
                        {
                                ++ crt_count;
                        }
                }
        }

        assert(count == thread::n_active_workers(m_workers));

        m_queue.m_condition.notify_all();
}

std::size_t thread::pool_t::n_workers() const
{
        return m_workers.size();
}

std::size_t thread::pool_t::n_active_workers() const
{
        const std::lock_guard<std::mutex> lock(m_queue.m_mutex);

        return thread::n_active_workers(m_workers);
}

std::size_t thread::pool_t::n_tasks() const
{
        const std::lock_guard<std::mutex> lock(m_queue.m_mutex);

        return m_queue.m_tasks.size();
}
