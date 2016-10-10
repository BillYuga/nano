#pragma once

#include "trainer.h"

namespace nano
{
        class stoch_optimizer_t;

        ///
        /// \brief stochastic trainer: each gradient update is computed for a random sub-set of samples.
        ///
        class stochastic_trainer_t : public trainer_t
        {
        public:

                NANO_MAKE_CLONABLE(stochastic_trainer_t,
                        "opt=sg[...],epochs=16[1,1024],policy=stop_early[,all_epochs],min_batch=32[32,1024],max_batch=256[32,4096]")

                // constructor
                explicit stochastic_trainer_t(const string_t& parameters = string_t());

                // train the model
                virtual trainer_result_t train(
                        const task_t&, const size_t fold, const size_t nthreads, const loss_t&, const criterion_t&,
                        model_t&) const override;
        };
}

