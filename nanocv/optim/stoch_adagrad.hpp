#pragma once

#include "nanocv/optim/stoch_params.hpp"
#include "nanocv/math/average_vector.hpp"

namespace ncv
{
        namespace optim
        {
                ///
                /// \brief stochastic AdaGrad
                ///     see "Adaptive subgradient methods for online learning and stochastic optimization"
                ///     by J. C. Duchi, E. Hazan, and Y. Singer
                ///
                ///     see http://xcorr.net/2014/01/23/adagrad-eliminating-learning-rates-in-stochastic-gradient-descent/
                ///
                template
                <
                        typename tproblem               ///< optimization problem
                >
                struct stoch_adagrad_t
                {
                        typedef stoch_params_t<tproblem>        param_t;
                        typedef typename param_t::tscalar       tscalar;
                        typedef typename param_t::tsize         tsize;
                        typedef typename param_t::tvector       tvector;
                        typedef typename param_t::tstate        tstate;
                        typedef typename param_t::twlog         twlog;
                        typedef typename param_t::telog         telog;
                        typedef typename param_t::tulog         tulog;

                        ///
                        /// \brief constructor
                        ///
                        stoch_adagrad_t(tsize epochs,
                                        tsize epoch_size,
                                        tscalar alpha0,
                                        tscalar decay,
                                        const twlog& wlog = twlog(),
                                        const telog& elog = telog(),
                                        const tulog& ulog = tulog())
                                :       m_param(epochs, epoch_size, alpha0, decay, wlog, elog, ulog)
                        {
                        }

                        ///
                        /// \brief minimize starting from the initial guess x0
                        ///
                        tstate operator()(const tproblem& problem, const tvector& x0) const
                        {
                                assert(problem.size() == static_cast<tsize>(x0.size()));

                                // current state
                                tstate cstate(problem, x0);

                                // running-weighted-averaged-per-dimension-squared gradient
                                average_vector_t<tscalar, tvector> gavg(x0.size());

                                for (tsize e = 0, k = 1; e < m_param.m_epochs; e ++)
                                {
                                        for (tsize i = 0; i < m_param.m_epoch_size; i ++, k ++)
                                        {
                                                // learning rate
                                                const tscalar alpha = m_param.m_alpha0;

                                                // descent direction                                                
                                                gavg.update(cstate.g.array().square(), m_param.weight(k));

                                                cstate.d = -cstate.g.array() /
                                                           (m_param.m_epsilon + gavg.value().array()).sqrt();

                                                // update solution
                                                cstate.update(problem, alpha);
                                        }

                                        m_param.ulog(cstate);
                                }

                                return cstate;
                        }

                        // attributes
                        param_t         m_param;
                };
        }
}

