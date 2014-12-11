#pragma once

#include "trainer.h"

namespace ncv
{
        ///
        /// batch trainer: each gradient update is computed for all samples.
        ///
        /// parameters:
        ///     opt=lbfgs[,cgd,gd]      - optimization method
        ///     iters=1024[4,4096]      - maximum number of iterations
        ///     eps=1e-6[1e-8,1e-3]     - convergence        
        ///
        class batch_trainer_t : public trainer_t
        {
        public:

                NANOCV_MAKE_CLONABLE(batch_trainer_t,
                                     "parameters: opt=lbfgs[,cgd,gd],iters=1024[4,4096],eps=1e-6[1e-8,1e-3]")

                // constructor
                batch_trainer_t(const string_t& parameters = string_t());

                // train the model
                virtual trainer_result_t train(
                        const task_t&, const fold_t&, const loss_t&, size_t nthreads, const string_t& criterion, 
                        model_t&) const;

        private:

                trainer_result_t batch_train(
                        const task_t&, const sampler_t& tsampler, const sampler_t& vsampler, size_t nthreads,
                        const loss_t&, const string_t& criterion, const model_t&) const;
        };
}
