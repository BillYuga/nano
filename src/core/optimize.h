#ifndef NANOCV_OPTIMIZE_H
#define NANOCV_OPTIMIZE_H

#include "stats.hpp"
#include "types.h"

namespace ncv
{
        namespace optimize
        {
                /////////////////////////////////////////////////////////////////////////////////////////////
                // optimization state:
                //      current point (x), function value (f), gradient (g),
                //      descent direction (d) & line-search step (t).
                ////////////////////////////////////////////////////////////////////////////////////////////////

                class problem_t;

                struct state_t
                {
                        // constructors
                        state_t(size_t size = 0);
                        state_t(const problem_t& problem, const vector_t& x0);

                        // update current point
                        void update(const problem_t& problem, scalar_t t);
                        void update(scalar_t t, scalar_t ft, const vector_t& gt);

                        // attributes
                        vector_t x, g, d;
                        scalar_t f, t;
                };

                // compare two optimization states
                inline bool operator<(const state_t& one, const state_t& other)
                {
                        return one.f < other.f;
                }

                /////////////////////////////////////////////////////////////////////////////////////////////
                // checks the convergence:
                //      the gradient is relatively small.
                /////////////////////////////////////////////////////////////////////////////////////////////

                inline bool converged(scalar_t epsilon, const state_t& st)
                {
                        return st.g.lpNorm<Eigen::Infinity>() < epsilon * (1.0 + std::fabs(st.f));
                }

                /////////////////////////////////////////////////////////////////////////////////////////////
                // optimization result:
                //      solution & statistics
                /////////////////////////////////////////////////////////////////////////////////////////////

                class result_t
                {
                public:

                        // constructor (analytic gradient)
                        explicit result_t(size_t size = 0);

                        // update solution
                        void update(const problem_t& problem, const state_t& st);

                        // access functions
                        const state_t& optimum() const { return m_optimum; }
                        const stats_t<scalar_t>& speed() const { return m_conv_speed; }
                        size_t iterations() const { return m_iterations; }
                        size_t n_fval_calls() const { return m_cnt_fval; }
                        size_t n_grad_calls() const { return m_cnt_grad; }

                private:

                        // attributes
                        state_t                 m_optimum;              // optimum state
                        size_t                  m_iterations;           // #iterations
                        stats_t<scalar_t>       m_conv_speed;           // convergence speed statistics
                        size_t                  m_cnt_fval;
                        size_t                  m_cnt_grad;
                };

                /////////////////////////////////////////////////////////////////////////////////////////////
                // describes a multivariate optimization problem.
                /////////////////////////////////////////////////////////////////////////////////////////////

                // dimensionality
                typedef std::function<size_t(void)>                                     op_size_t;

                // function value
                typedef std::function<scalar_t(const vector_t&)>                        op_fval_t;

                // function value & gradient
                typedef std::function<scalar_t(const vector_t&, vector_t&)>             op_fval_grad_t;

                class problem_t
                {
                public:

                        // constructor (analytic gradient)
                        explicit problem_t(
                                const op_size_t& op_size,
                                const op_fval_t& op_fval,
                                const op_fval_grad_t& op_fval_grad);

                        // constructor (no analytic gradient, can be estimated)
                        explicit problem_t(
                                const op_size_t& op_size,
                                const op_fval_t& op_fval);

                        // compute dimensionality & function value & gradient
                        size_t size() const;
                        scalar_t f(const vector_t& x) const;
                        scalar_t f(const vector_t& x, vector_t& g) const;

                        // access functions
                        size_t n_fval_calls() const { return m_cnt_fval; }
                        size_t n_grad_calls() const { return m_cnt_grad; }

                private:

                        // evaluate gradient (if not provided)
                        void eval_grad(const vector_t x, vector_t& g) const;

                private:

                        // attributes
                        op_size_t               m_op_size;
                        op_fval_t               m_op_fval;
                        op_fval_grad_t          m_op_fval_grad;
                        mutable size_t          m_cnt_fval;
                        mutable size_t          m_cnt_grad;
                };

                // result updated
                typedef std::function<void(const result_t&)>    op_updated_t;

                /////////////////////////////////////////////////////////////////////////////////////////////
                // check and force a descent direction.
                /////////////////////////////////////////////////////////////////////////////////////////////

                scalar_t descent(state_t& st);

                /////////////////////////////////////////////////////////////////////////////////////////////
                // line-search methods to find the scalar that reduces
                //      the function value (the most) along the direction d:
                //
                //      argmin(t) f(x + t * d).
                /////////////////////////////////////////////////////////////////////////////////////////////

                // Armijo (sufficient decrease) condition
                scalar_t ls_armijo(const problem_t& problem, state_t& st,
                        scalar_t alpha = 0.2, scalar_t beta = 0.7, size_t max_iters = 64);

                // helper function: strong Wolfe (sufficient decrease and curvature) conditions
                scalar_t ls_zoom(const problem_t& problem, const state_t& st,
                        scalar_t& ft, vector_t& gt,
                        scalar_t tlo, scalar_t thi, scalar_t ftlo, scalar_t fthi,
                        scalar_t c1, scalar_t c2, size_t max_iters = 64);

                // strong Wolfe (sufficient decrease and curvature) conditions
                scalar_t ls_strong_wolfe(const problem_t& problem, state_t& st,
                        scalar_t& ft, vector_t& gt,
                        scalar_t c1 = 1e-4, scalar_t c2 = 0.1, size_t max_iters = 64);

                /////////////////////////////////////////////////////////////////////////////////////////////
                // gradient descent starting from the initial value (guess) x0.
                /////////////////////////////////////////////////////////////////////////////////////////////

                result_t gd(
                        const problem_t& problem,
                        const vector_t& x0,
                        size_t max_iterations,          // maximum number of iterations
                        scalar_t epsilon,               // convergence precision
                        const op_updated_t& op_updated = op_updated_t());

                /////////////////////////////////////////////////////////////////////////////////////////////
                // conjugate gradient descent starting from the initial value (guess) x0.
                /////////////////////////////////////////////////////////////////////////////////////////////

                result_t cgd(
                        const problem_t& problem,
                        const vector_t& x0,
                        size_t max_iterations,          // maximum number of iterations
                        scalar_t epsilon,               // convergence precision
                        const op_updated_t& op_updated = op_updated_t());

                /////////////////////////////////////////////////////////////////////////////////////////////
                // limited memory bfgs (l-bfgs) starting from the initial value (guess) x0.
                /////////////////////////////////////////////////////////////////////////////////////////////

                result_t lbfgs(
                        const problem_t& problem,
                        const vector_t& x0,
                        size_t max_iterations,          // maximum number of iterations
                        scalar_t epsilon,               // convergence precision
                        size_t history_size = 8,        // hessian approximation history size
                        const op_updated_t& op_updated = op_updated_t());
        }
}

#endif // NANOCV_OPTIMIZE_H
