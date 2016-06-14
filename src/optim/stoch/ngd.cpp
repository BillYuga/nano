#include "ngd.h"
#include "loop.hpp"

namespace nano
{
        state_t stoch_ngd_t::operator()(const stoch_params_t& param, const problem_t& problem, const vector_t& x0) const
        {
                const auto op = [&] (const auto... hypers)
                {
                        return this->operator()(param, problem, x0, hypers...);
                };

                return nano::tune(op, make_alpha0s()).optimum();
        }

        state_t stoch_ngd_t::operator()(const stoch_params_t& param, const problem_t& problem, const vector_t& x0,
                const scalar_t alpha0) const
        {
                assert(problem.size() == x0.size());

                const auto op_iter = [&] (state_t& cstate)
                {
                        // learning rate
                        const scalar_t alpha = alpha0;

                        // descent direction
                        const scalar_t norm = 1 / cstate.g.template lpNorm<2>();
                        cstate.d = -cstate.g * norm;

                        // update solution
                        cstate.update(problem, alpha);
                };

                // OK, assembly the optimizer
                return  stoch_loop(problem, param, state_t(problem, x0), op_iter,
                        {{"alpha0", alpha0}});
        }
}
