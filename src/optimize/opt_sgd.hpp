#ifndef NANOCV_OPTIMIZE_OPTIMIZER_SGD_HPP
#define NANOCV_OPTIMIZE_OPTIMIZER_SGD_HPP

#include <cassert>
#include <cmath>

namespace ncv
{
        namespace optimize
        {
                /////////////////////////////////////////////////////////////////////////////////////////
                // stochastic gradient descent starting from the initial value (guess) x0.
                /////////////////////////////////////////////////////////////////////////////////////////

                template
                <
                        typename tscalar,
                        typename tsize
                >
                tscalar sgd_learning_rate(tscalar gamma, tscalar lambda, tsize iteration)
                {
                        // learning rate recommended by Bottou
                        return gamma / (1 + gamma * lambda * iteration);
                }

                template
                <
                        typename tproblem,

                        // dependent types
                        typename tscalar = typename tproblem::tscalar,
                        typename tsize = typename tproblem::tsize,
                        typename tvector = typename tproblem::tvector,
                        typename tresult = typename tproblem::tresult,
                        typename tstate = typename tproblem::tstate
                >
                tresult sgd(
                        const tproblem& problem,
                        const tvector& x0,
                        tsize iterations,
                        tscalar gamma,
                        tscalar lambda)
                {
                        assert(problem.size() == static_cast<tsize>(x0.size()));

                        tvector x = x0, g = x;

                        // (A=average)SGD steps
                        for (tsize i = 0; i < iterations; i ++)
                        {
                                const tscalar f = problem(x, g);
                                const tscalar d = sgd_learning_rate(gamma, lambda, i);

                                if (    !std::isinf(f) && !std::isinf(g.minCoeff()) && !std::isinf(g.maxCoeff()) &&
                                        !std::isnan(f) && !std::isnan(g.minCoeff()) && !std::isnan(g.maxCoeff()))
                                {
                                        x.noalias() -= d * g;
                                }
                        }

                        // evaluate solution
                        tstate cstate(problem.size());
                        cstate.x = x;
                        cstate.f = problem(x);

                        tresult result(problem.size());
                        result.update(problem, cstate);
                        return result;
                }
        }
}

#endif // NANOCV_OPTIMIZE_OPTIMIZER_SGD_HPP
