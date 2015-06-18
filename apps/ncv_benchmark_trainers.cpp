#include "nanocv/nanocv.h"
#include "nanocv/measure.hpp"
#include "nanocv/tabulator.h"
#include "nanocv/accumulator.h"
#include "nanocv/thread/thread.h"
#include "nanocv/trainers/batch.h"
#include "nanocv/trainers/minibatch.h"
#include "nanocv/trainers/stochastic.h"
#include "nanocv/tasks/task_synthetic_shapes.h"

using namespace ncv;

static string_t stats_to_string(const stats_t<scalar_t>& stats)
{
        return  text::to_string(stats.avg())
                + "+/-" + text::to_string(stats.stdev())
                + " [" + text::to_string(stats.min())
                + ", " + text::to_string(stats.max())
                + "]";
}

template
<
        typename ttrainer
>
static void test_optimizer(model_t& model, const string_t& name, tabulator_t& table, const vectors_t& x0s,
        ttrainer trainer)
{
        stats_t<scalar_t> terrors;
        stats_t<scalar_t> verrors;

        log_info() << "<<< running " << name << " ...";

        const ncv::timer_t timer;

        for (const vector_t& x0 : x0s)
        {
                model.load_params(x0);

                const trainer_result_t result = trainer();
                const trainer_state_t state = result.optimum_state();

                terrors(state.m_terror_avg);
                verrors(state.m_verror_avg);

                log_info() << "<<< " << name << ", optimum config = {" << text::concatenate(result.optimum_config())
                           << "}, optimum epoch = " << result.optimum_epoch()
                           << ", error " << state.m_terror_avg << "/" << state.m_verror_avg << ".";
        }

        table.append(name)
                << stats_to_string(terrors)
                << stats_to_string(verrors)
                << (timer.seconds());
}

static void test_optimizers(
        const task_t& task, model_t& model, const sampler_t& tsampler, const sampler_t& vsampler,
        const loss_t& loss, const string_t& criterion, tabulator_t& table)
{
        const size_t cmd_trials = 16;

        const size_t cmd_iterations = 64;
        const size_t cmd_minibatch_epochs = cmd_iterations;
        const size_t cmd_stochastic_epochs = cmd_iterations;
        const scalar_t cmd_epsilon = 1e-4;

        const size_t n_threads = ncv::n_threads();
        const bool verbose = true;

        // generate fixed random starting points
        vectors_t x0s(cmd_trials);
        for (vector_t& x0 : x0s)
        {
                model.random_params();
                model.save_params(x0);
        }

        // batch optimizers
        const auto batch_optimizers =
        {
                optim::batch_optimizer::GD,
                optim::batch_optimizer::CGD,
                optim::batch_optimizer::LBFGS
        };

        // minibatch optimizers
        const auto minibatch_optimizers =
        {
                optim::batch_optimizer::GD,
                optim::batch_optimizer::CGD,
                optim::batch_optimizer::LBFGS
        };

        // stochastic optimizers
        const auto stoch_optimizers =
        {
                optim::stoch_optimizer::SG,
                optim::stoch_optimizer::SGA,
                optim::stoch_optimizer::SIA,
                optim::stoch_optimizer::AG,
                optim::stoch_optimizer::AGGR,
                optim::stoch_optimizer::ADAGRAD,
                optim::stoch_optimizer::ADADELTA
        };

        const string_t basename = "[" + text::to_string(criterion) + "] ";

        // run optimizers and collect results
        for (optim::batch_optimizer optimizer : batch_optimizers)
        {
                test_optimizer(model, basename + "batch-" + text::to_string(optimizer), table, x0s, [&] ()
                {
                        return ncv::batch_train(
                                model, task, tsampler, vsampler, n_threads,
                                loss, criterion, optimizer, cmd_iterations, cmd_epsilon, verbose);
                });
        }

        for (optim::batch_optimizer optimizer : minibatch_optimizers)
        {
                test_optimizer(model, basename + "minibatch-" + text::to_string(optimizer), table, x0s, [&] ()
                {
                        return ncv::minibatch_train(
                                model, task, tsampler, vsampler, n_threads,
                                loss, criterion, optimizer, cmd_minibatch_epochs, cmd_epsilon, verbose);
                });
        }

        for (optim::stoch_optimizer optimizer : stoch_optimizers)
        {
                test_optimizer(model, basename + "stochastic-" + text::to_string(optimizer), table, x0s, [&] ()
                {
                        return ncv::stochastic_train(
                                model, task, tsampler, vsampler, n_threads,
                                loss, criterion, optimizer, cmd_stochastic_epochs, verbose);
                });
        }
}

int main(int, char* [])
{
        ncv::init();

        const size_t cmd_rows = 16;
        const size_t cmd_cols = 16;
        const size_t cmd_outputs = 2;
        const size_t cmd_samples = ncv::n_threads() * 16 * 10;
        const color_mode cmd_color = color_mode::rgba;

        // create task
        synthetic_shapes_task_t task(cmd_rows, cmd_cols, cmd_outputs, cmd_color, cmd_samples);
        task.load("");
	task.describe();

        // create training & validation samples
        sampler_t tsampler(task);
        tsampler.setup(sampler_t::atype::annotated);

        sampler_t vsampler(task);
        tsampler.split(80, vsampler);

        // construct models
//        const string_t lmodel0;
//        const string_t lmodel1 = lmodel0 + "linear:dims=64;act-snorm;";
//        const string_t lmodel2 = lmodel1 + "linear:dims=32;act-snorm;";
//        const string_t lmodel3 = lmodel2 + "linear:dims=16;act-snorm;";

        string_t cmodel;
        cmodel = cmodel + "conv:dims=16,rows=5,cols=5;pool-max;act-snorm;";
        cmodel = cmodel + "conv:dims=32,rows=3,cols=3;act-snorm;";

        const string_t outlayer = "linear:dims=" + text::to_string(cmd_outputs) + ";";

        strings_t cmd_networks =
        {
//                lmodel0 + outlayer,
//                lmodel1 + outlayer,
//                lmodel2 + outlayer,
//                lmodel3 + outlayer,

                cmodel + outlayer
        };

        const strings_t cmd_losses = { "classnll" };    //ncv::get_losses().ids();
        const strings_t cmd_criteria = { "avg" }; //ncv::get_criteria().ids();

        // vary the model
        for (const string_t& cmd_network : cmd_networks)
        {
                log_info() << "<<< running network [" << cmd_network << "] ...";

                const rmodel_t model = ncv::get_models().get("forward-network", cmd_network);
                assert(model);
                model->resize(task, true);

                // vary the loss
                for (const string_t& cmd_loss : cmd_losses)
                {
                        log_info() << "<<< running loss [" << cmd_loss << "] ...";

                        const rloss_t loss = ncv::get_losses().get(cmd_loss);
                        assert(loss);

                        tabulator_t table("optimizer\\");
                        table.header() << "train error"
                                       << "valid error"
                                       << "time [msec]";

                        // vary the criteria
                        for (const string_t& cmd_criterion : cmd_criteria)
                        {
                                test_optimizers(task, *model, tsampler, vsampler, *loss, cmd_criterion, table);
                        }

                        // show results
                        table.print(std::cout);
                }

                log_info();
        }

        // OK
        log_info() << done;
        return EXIT_SUCCESS;
}
