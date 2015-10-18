#pragma once

#include "trainer_data.h"
#include "trainer_result.h"

namespace cortex
{
        class model_t;

        ///
        /// \brief minibatch train the given model
        ///
        NANOCV_PUBLIC trainer_result_t minibatch_train(
                const model_t&, const task_t&, const sampler_t& tsampler, const sampler_t& vsampler, size_t nthreads,
                const loss_t&, const string_t& criterion,
                min::batch_optimizer optimizer, size_t epochs, scalar_t epsilon, bool verbose = true);
}
