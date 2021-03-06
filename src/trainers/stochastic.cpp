#include "loop.h"
#include "model.h"
#include "stochastic.h"
#include "math/numeric.h"
#include "text/to_params.h"
#include "stoch_optimizer.h"
#include "text/from_params.h"
#include "trainer_function.h"

namespace nano
{
        stochastic_trainer_t::stochastic_trainer_t(const string_t& parameters) :
                trainer_t(to_params(parameters, "opt", "sg[...]", "epochs", "16[1,1024]",
                "policy", "stop_early[,all_epochs]", "min_batch", "32[32,1024]", "max_batch", "256[32,4096]", "eps", 1e-6, "patience", 32))
        {
        }

        trainer_result_t stochastic_trainer_t::train(
                const task_t& task, const size_t fold, const size_t nthreads,
                const loss_t& loss, const criterion_t& criterion,
                model_t& model) const
        {
                if (model != task)
                {
                        throw std::runtime_error("stochastic trainer: mis-matching model and task");
                }

                // parameters
                const auto epochs = clamp(from_params<size_t>(config(), "epochs"), 1, 1024);
                const auto policy = from_params<trainer_policy>(config(), "policy");
                const auto batch0 = clamp(from_params<size_t>(config(), "min_batch"), 32, 1024);
                const auto batchK = clamp(from_params<size_t>(config(), "max_batch"), batch0, 4096);
                const auto epsilon = from_params<scalar_t>(config(), "eps");
                const auto optimizer = from_params<string_t>(config(), "opt");
                const auto patience = from_params<size_t>(config(), "patience");

                const auto train_fold = fold_t{fold, protocol::train};
                const auto train_size = task.n_samples(train_fold);
                const auto samples = epochs * train_size;

                const auto factor = (batch0 == batchK) ?
                        scalar_t(1) :
                        clamp(scalar_t(samples - batch0) / scalar_t(samples - batchK), scalar_t(1), scalar_t(2));
                const auto epoch_size = (batch0 == batchK) ?
                        batch0 :
                        idiv(static_cast<size_t>(std::log(batchK / batch0) / std::log(factor)), epochs);

                // train the model
                const timer_t timer;
                const auto op = [&] (const accumulator_t& acc, const vector_t& x0)
                {
                        size_t epoch = 0;
                        trainer_result_t result;

                        task_iterator_t it(task, train_fold, batch0, factor);

                        // tuning operator
                        const auto fn_tlog = [&] (const state_t& state, const string_t& sconfig)
                        {
                                // NB: the training state is already computed
                                const auto train = trainer_measurement_t{acc.value(), acc.vstats(), acc.estats()};
                                const auto config = to_params(sconfig, "lambda", acc.lambda());

                                log_info()
                                        << "[tune:train=" << train
                                        << "," << config << ",batch=" << it.size() << ",g=" << state.convergence_criteria()
                                        << "] " << timer.elapsed() << ".";

                                // NB: need to reset the minibatch size (changed during tuning)!
                                it.reset(batch0, factor);

                                return train.m_value;
                        };

                        // logging operator
                        const auto fn_ulog = [&] (const state_t& state, const string_t& sconfig)
                        {
                                 return ulog(acc, it, epoch, epochs, result, policy, patience, timer, state, sconfig);
                        };

                        // assembly optimization function & optimize the model
                        const auto function = trainer_function_t(acc, it);
                        const auto params = stoch_params_t{epochs, epoch_size, epsilon, fn_ulog, fn_tlog};

                        get_stoch_optimizers().get(optimizer)->minimize(params, function, x0);

                        return result;
                };

                const auto result = trainer_loop(model, nthreads, loss, criterion, op);
                log_info() << "<<< stoch-" << optimizer << ": " << result << ",time=" << timer.elapsed() << ".";

                // OK
                if (result.valid())
                {
                        model.load_params(result.optimum_params());
                }
                return result;
        }
}
