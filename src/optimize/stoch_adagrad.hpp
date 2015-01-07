#pragma once

#include "stoch_params.hpp"
#include "average.hpp"
#include <cassert>
#include <limits>

namespace ncv
{
        namespace optimize
        {
                ///
                /// \brief stochastic AdaGrad
                ///
                /// NB: "Adaptive subgradient methods for online learning and stochastic optimization"
                ///     -  J. C. Duchi, E. Hazan, and Y. Singer
                /// NB: http://xcorr.net/2014/01/23/adagrad-eliminating-learning-rates-in-stochastic-gradient-descent/
                ///
                template
                <
                        typename tproblem               ///< optimization problem
                >
                struct stoch_adagrad : public stoch_params_t<tproblem>
                {
                        typedef stoch_params_t<tproblem>        base_t;

                        typedef typename base_t::tscalar        tscalar;
                        typedef typename base_t::tsize          tsize;
                        typedef typename base_t::tvector        tvector;
                        typedef typename base_t::tstate         tstate;
                        typedef typename base_t::twlog          twlog;
                        typedef typename base_t::telog          telog;
                        typedef typename base_t::tulog          tulog;

                        ///
                        /// \brief constructor
                        ///
                        stoch_adagrad(  tsize epochs,
                                        tsize epoch_size,
                                        tscalar alpha0,
                                        tscalar decay,
                                        const twlog& wlog = twlog(),
                                        const telog& elog = telog(),
                                        const tulog& ulog = tulog())
                                :       base_t(epochs, epoch_size, alpha0, decay, wlog, elog, ulog)
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
                                average_vector<tscalar, tvector> gavg(x0.size());

                                //
                                const tscalar epsilon = std::sqrt(std::numeric_limits<tscalar>::epsilon());

                                for (tsize e = 0, k = 1; e < base_t::m_epochs; e ++)
                                {
                                        for (tsize i = 0; i < base_t::m_epoch_size; i ++, k ++)
                                        {
                                                // learning rate
                                                const tscalar alpha = base_t::m_alpha0;

                                                // descent direction                                                
                                                gavg.update(cstate.g.array().square(), tscalar(k));
                                                cstate.d = -cstate.g.array() / (epsilon + gavg.value().array()).sqrt();

                                                // update solution
                                                cstate.update(problem, alpha);
                                        }

                                        base_t::ulog(cstate);
                                }

                                return cstate;
                        }
                };
        }
}

