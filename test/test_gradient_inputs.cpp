#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_gradient_inputs"

#include <boost/test/unit_test.hpp>
#include "libnanocv/nanocv.h"
#include "libnanocv/accumulator.h"
#include "libnanocv/util/logger.h"
#include "libnanocv/util/close.hpp"
#include "libnanocv/util/random.hpp"
#include "libnanocv/util/epsilon.hpp"
#include "libnanocv/util/thread_pool.h"
#include "libnanocv-test/check_gradients.h"

namespace test
{
        using namespace ncv;

        thread_pool_t::mutex_t mutex;

        size_t n_checks = 0;
        size_t n_failures1 = 0;
        size_t n_failures2 = 0;
        size_t n_failures3 = 0;

        void test_grad(const string_t& header, const string_t& loss_id, const model_t& model)
        {
                rmodel_t rmodel_inputs = model.clone();

                const size_t n_tests = 256;

                const rloss_t rloss = loss_manager_t::instance().get(loss_id);
                const loss_t& loss = *rloss;

                const size_t psize = model.psize();
                const size_t isize = model.isize();
                const size_t osize = model.osize();

                vector_t params(psize);
                vector_t target(osize);
                tensor_t input(model.idims(), model.irows(), model.icols());

                // optimization problem (wrt parameters & inputs): size
                auto fn_inputs_size = [&] ()
                {
                        return isize;
                };

                // optimization problem (wrt parameters & inputs): function value
                auto fn_inputs_fval = [&] (const vector_t& x)
                {
                        rmodel_inputs->load_params(params);

                        const vector_t output = rmodel_inputs->output(x).vector();

                        return loss.value(target, output);
                };

                // optimization problem (wrt parameters & inputs): function value & gradient
                auto fn_inputs_grad = [&] (const vector_t& x, vector_t& gx)
                {
                        rmodel_inputs->load_params(params);

                        const vector_t output = rmodel_inputs->output(x).vector();

                        gx = rmodel_inputs->ginput(loss.vgrad(target, output)).vector();
                        return loss.value(target, output);
                };

                // construct optimization problem: analytic gradient and finite difference approximation
                const opt_problem_t problem_analytic_inputs(fn_inputs_size, fn_inputs_fval, fn_inputs_grad);
                const opt_problem_t problem_aproxdif_inputs(fn_inputs_size, fn_inputs_fval);

                for (size_t t = 0; t < n_tests; t ++)
                {
                        random_t<scalar_t> prgen(-1.0, +1.0);
                        random_t<scalar_t> irgen(-0.1, +0.1);
                        random_t<size_t> trgen(0, osize - 1);

                        prgen(params.data(), params.data() + psize);
                        target = ncv::class_target(trgen(), osize);
                        irgen(input.data(), input.data() + isize);

                        vector_t analytic_inputs_grad, aproxdif_inputs_grad;

                        problem_analytic_inputs(input.vector(), analytic_inputs_grad);
                        problem_aproxdif_inputs(input.vector(), aproxdif_inputs_grad);

                        const scalar_t delta = (analytic_inputs_grad - aproxdif_inputs_grad).lpNorm<Eigen::Infinity>();

                        // update statistics
                        const thread_pool_t::lock_t lock(test::mutex);

                        n_checks ++;
                        if (!math::close(delta, scalar_t(0), math::epsilon1<scalar_t>()))
                        {
                                n_failures1 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon2<scalar_t>()))
                        {
                                n_failures2 ++;
                        }
                        if (!math::close(delta, scalar_t(0), math::epsilon3<scalar_t>()))
                        {
                                n_failures3 ++;

                                BOOST_CHECK_LE(delta, math::epsilon3<scalar_t>());

                                log_error() << header << ": error = " << delta << "/" << math::epsilon3<scalar_t>() << "!";
                        }
                }
        }
}

BOOST_AUTO_TEST_CASE(test_gradient_inputs)
{
        using namespace ncv;

        ncv::init();

        size_t cmd_irows;
        size_t cmd_icols;
        size_t cmd_outputs;
        color_mode cmd_color;

        auto configs = test::make_grad_configs(cmd_irows, cmd_icols, cmd_outputs, cmd_color);

        // test each configuration
        thread_pool_t pool;
        for (auto config : configs)
        {
                pool.enqueue([=] ()
                {
                        const string_t desc = config.first;
                        const string_t loss_id = config.second;

                        // create model
                        const rmodel_t model = model_manager_t::instance().get("forward-network", desc);
                        BOOST_CHECK_EQUAL(model.operator bool(), true);
                        {
                                const thread_pool_t::lock_t lock(test::mutex);

                                model->resize(cmd_irows, cmd_icols, cmd_outputs, cmd_color, false);
                        }

                        // check with the given loss
                        test::test_grad("[loss = " + loss_id + "]", loss_id, *model);
                });
        };

        pool.wait();

        // print statistics
        const scalar_t eps1 = math::epsilon1<scalar_t>();
        const scalar_t eps2 = math::epsilon2<scalar_t>();
        const scalar_t eps3 = math::epsilon3<scalar_t>();

        log_info() << "failures: level1 = " << test::n_failures1 << "/" << test::n_checks << ", epsilon = " << eps1;
        log_info() << "failures: level2 = " << test::n_failures2 << "/" << test::n_checks << ", epsilon = " << eps2;
        log_info() << "failures: level3 = " << test::n_failures3 << "/" << test::n_checks << ", epsilon = " << eps3;

        BOOST_CHECK_LE(test::n_failures1 * 100, 100 * test::n_checks);
        BOOST_CHECK_LE(test::n_failures2 * 100, 5 * test::n_checks);
        BOOST_CHECK_LE(test::n_failures3 * 100, 0 * test::n_checks);
}