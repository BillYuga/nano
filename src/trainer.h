#pragma once

#include "manager.h"
#include "trainer_result.h"

namespace nano
{
        class loss_t;
        class task_t;
        class model_t;
        class sampler_t;
        class criterion_t;
        class accumulator_t;

        ///
        /// \brief stores registered prototypes
        ///
        class trainer_t;
        using trainer_manager_t = manager_t<trainer_t>;
        using rtrainer_t = trainer_manager_t::trobject;

        NANO_PUBLIC trainer_manager_t& get_trainers();

        ///
        /// \brief generic trainer: optimizes a model on a given compatible task.
        ///
        class NANO_PUBLIC trainer_t : public clonable_t
        {
        public:

                ///
                /// \brief constructor
                ///
                explicit trainer_t(const string_t& configuration) : clonable_t(configuration) {}

                ///
                /// \brief train the given model starting from the current model parameters
                ///
                virtual trainer_result_t train(
                        const task_t&, const size_t fold, const size_t nthreads,
                        const loss_t&, const criterion_t& criterion,
                        model_t&) const = 0;
        };
}

