#pragma once

#include "loop.hpp"
#include "ls_init.hpp"
#include "ls_strategy.hpp"
#include <deque>

namespace nano
{
        ///
        /// \brief limited memory bfgs (l-bfgs)
        ///
        struct batch_lbfgs_t
        {
                ///
                /// \brief minimize starting from the initial guess x0
                ///
                state_t operator()(const batch_params_t& param, const problem_t& problem, const vector_t& x0) const
                {
                        assert(problem.size() == x0.size());

                        std::deque<vector_t> ss, ys;
                        state_t istate(problem, x0);             // initial state
                        state_t pstate = istate;                 // previous state

                        vector_t q, r;

                        // line-search initial step length
                        ls_init_t ls_init(param.m_ls_initializer);

                        // line-search step
                        ls_strategy_t ls_step(param.m_ls_strategy, scalar_t(1e-4), scalar_t(0.9));

                        const auto op = [&] (state_t& cstate, const std::size_t i)
                        {
                                // descent direction
                                //      (see "Numerical optimization", Nocedal & Wright, 2nd edition, p.178)
                                q = cstate.g;

                                auto itr_s = ss.rbegin();
                                auto itr_y = ys.rbegin();
                                std::vector<scalar_t> alphas;
                                for (std::size_t j = 1; j <= param.m_hsize && i >= j; j ++)
                                {
                                        const vector_t& s = (*itr_s ++);
                                        const vector_t& y = (*itr_y ++);

                                        const scalar_t alpha = s.dot(q) / s.dot(y);
                                        q.noalias() -= alpha * y;
                                        alphas.push_back(alpha);
                                }

                                if (i == 0)
                                {
                                        r = q;
                                }
                                else
                                {
                                        const vector_t& s = *ss.rbegin();
                                        const vector_t& y = *ys.rbegin();
                                        r = s.dot(y) / y.dot(y) * q;
                                }

                                auto it_s = ss.begin();
                                auto it_y = ys.begin();
                                auto itr_alpha = alphas.rbegin();
                                for (std::size_t j = 1; j <= param.m_hsize && i >= j; j ++)
                                {
                                        const vector_t& s = (*it_s ++);
                                        const vector_t& y = (*it_y ++);

                                        const scalar_t alpha = *(itr_alpha ++);
                                        const scalar_t beta = y.dot(r) / s.dot(y);
                                        r.noalias() += s * (alpha - beta);
                                }

                                cstate.d = -r;

                                // line-search
                                pstate = cstate;

                                const scalar_t t0 = ls_init(cstate);
                                if (!ls_step(problem, t0, cstate))
                                {
                                        return false;
                                }

                                ss.push_back(cstate.x - pstate.x);
                                ys.push_back(cstate.g - pstate.g);
                                if (ss.size() > param.m_hsize)
                                {
                                        ss.pop_front();
                                        ys.pop_front();
                                }

                                return true;
                        };

                        // OK, assembly the optimizer
                        return batch_loop(param, istate, op);
                }
        };
}
