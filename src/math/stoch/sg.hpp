#pragma once

#include "lrate.hpp"
#include "stoch_loop.hpp"

namespace zob
{
        ///
        /// \brief stochastic gradient (descent)
        ///     see "Minimizing Finite Sums with the Stochastic Average Gradient",
        ///     by Mark Schmidth, Nicolas Le Roux, Francis Bach
        ///
        template
        <
                typename tproblem               ///< optimization problem
        >
        struct stoch_sg_t
        {
                using param_t = stoch_params_t<tproblem>;
                using tstate = typename param_t::tstate;
                using tscalar = typename param_t::tscalar;
                using tvector = typename param_t::tvector;

                ///
                /// \brief minimize starting from the initial guess x0
                ///
                tstate operator()(const param_t& param, const tproblem& problem, const tvector& x0) const
                {
                        const auto op = [&] (const auto... params)
                        {
                                return this->operator()(param.tunable(), problem, x0, params...);
                        };

                        const auto config = zob::tune(op, make_alpha0s(), make_decays());
                        return operator()(param, problem, x0, config.param0(), config.param1());
                }

                ///
                /// \brief minimize starting from the initial guess x0
                ///
                tstate operator()(const param_t& param, const tproblem& problem, const tvector& x0,
                        const tscalar alpha0, const tscalar decay) const
                {
                        assert(problem.size() == x0.size());

                        // learning rate schedule
                        lrate_t<tscalar> lrate(alpha0, decay);

                        const auto op_iter = [&] (tstate& cstate, const std::size_t iter)
                        {
                                // learning rate
                                const tscalar alpha = lrate.get(iter);

                                // descent direction
                                cstate.d = -cstate.g;

                                // update solution
                                cstate.update(problem, alpha);
                        };

                        // OK, assembly the optimizer
                        return  stoch_loop(problem, param, tstate(problem, x0), op_iter,
                                {{"alpha0", alpha0}, {"decay", decay}});
                }
        };
}

