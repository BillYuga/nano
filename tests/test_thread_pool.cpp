#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_thread_pool"

#include <boost/test/unit_test.hpp>
#include "nanocv/logger.h"
#include "nanocv/thread/pool.h"
#include "nanocv/math/random.hpp"

BOOST_AUTO_TEST_CASE(test_thread_pool)
{
        using namespace ncv;

        thread_pool_t pool;

        thread_pool_t::mutex_t mutex;

        // check that there is no job to do
        BOOST_CHECK_EQUAL(pool.n_workers(), ncv::n_threads());
        BOOST_CHECK_EQUAL(pool.n_tasks(), 0);

        const size_t n_tests = 8;
        const size_t n_max_jobs = pool.n_workers() * 16;

        // run multiple tests ...
        for (size_t t = 0; t < n_tests; t ++)
        {
                random_t<size_t> rnd(1, n_max_jobs);

                // ... enqueue jobs
                const size_t n_tasks = rnd();
                log_info() << "@pool [" << (t + 1) << "/" << n_tests
                           << "]: creating " << n_tasks << " jobs ...";

                for (size_t j = 0; j < n_tasks; j ++)
                {
                        pool.enqueue([=, &mutex]()
                        {
                                const size_t sleep1 = random_t<size_t>(5, 20)();
                                std::this_thread::sleep_for(std::chrono::milliseconds(sleep1));

                                {
                                        const thread_pool_t::lock_t lock(mutex);

                                        log_info() << "#job [" << (j + 1) << "/" << n_tasks << "@"
                                                   << (t + 1) << "/" << n_tests << "] started ...";
                                }

                                const size_t sleep2 = random_t<size_t>(5, 50)();
                                std::this_thread::sleep_for(std::chrono::milliseconds(sleep2));

                                {
                                        const thread_pool_t::lock_t lock(mutex);

                                        log_info() << "#job [" << (j + 1) << "/" << n_tasks << "@"
                                                   << (t + 1) << "/" << n_tests << "] done.";
                                }
                        });
                }

                // ... wait for all jobs to finish
                log_info() << "@pool [" << (t + 1) << "/" << n_tests
                           << "]: waiting for " << pool.n_tasks() << " jobs ...";

                pool.wait();

                log_info() << "@pool [" << (t + 1) << "/" << n_tests
                                << "]: waiting done (enqueued " << pool.n_tasks() << " jobs).";

                // check that all jobs are done
                BOOST_CHECK_EQUAL(pool.n_workers(), ncv::n_threads());
                BOOST_CHECK_EQUAL(pool.n_tasks(), 0);
        }
}
