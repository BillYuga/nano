#pragma once

#include <utility>
#include <cassert>

namespace ncv
{
        namespace optimize
        {
                ///
                /// \brief stochastic iterative average gradient (descent) starting from the initial value (guess) x0
                ///
                /// NB: "Minimizing Finite Sums with the Stochastic Average Gradient"
                ///     - Mark Schmidth, Nicolas Le Roux, Francis Bach
                ///
                template
                <
                        typename tproblem,

                        // dependent types
                        typename tscalar = typename tproblem::tscalar,
                        typename tsize = typename tproblem::tsize,
                        typename tvector = typename tproblem::tvector,
                        typename tstate = typename tproblem::tstate,

                        typename twlog = typename tproblem::twlog,
                        typename telog = typename tproblem::telog,
                        typename tulog = typename tproblem::tulog
                >
                std::pair<tvector, tscalar> stoch_sia(
                        const tproblem& problem,
                        const tvector& x0,
                        tsize max_iterations,           ///< maximum number of iterations
                        tscalar alpha0,                 ///< initial learning rate
                        tscalar beta)                   ///< decreasing factor for the learning rate (<1)
                {
                        assert(problem.size() == static_cast<tsize>(x0.size()));

                        tvector x = x0, xavg = x;
                        tvector g;

                        tscalar alpha = alpha0;
                        tscalar sumb = tscalar(1) / alpha;

                        for (tsize i = 0; i < max_iterations; i ++, alpha *= beta)
                        {
                                problem(x, g);

                                x.noalias() -= alpha * g;

                                // average parameters
                                const tscalar b = tscalar(1) / alpha;
                                xavg = (xavg * sumb + x * b) / (sumb + b);
                                sumb = sumb + b;
                        }

                        // OK, return <optimum parameters, last learning rate>
                        return std::make_pair(xavg, alpha);
                }
        }
}

