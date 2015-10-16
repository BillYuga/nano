#pragma once

#include "function.hpp"

namespace func
{
        ///
        /// \brief create Zakharov test functions
        ///
        template
        <
                typename tscalar
        >
        struct function_zakharov_t : public function_t<tscalar>
        {
                typedef typename function_t<tscalar>::tsize     tsize;
                typedef typename function_t<tscalar>::tvector   tvector;
                typedef typename function_t<tscalar>::tproblem  tproblem;  
                
                explicit function_zakharov_t(const tsize dims)
                        :       m_dims(dims)
                {
                }

                virtual std::string name() const override
                {
                        return "Zakharov" + std::to_string(m_dims) + "D";
                }

                virtual tproblem problem() const override
                {
                        const auto fn_size = [=] ()
                        {
                                return m_dims;
                        };

                        const auto fn_fval = [=] (const tvector& x)
                        {
                                tscalar u = 0, v = 0;
                                for (tsize i = 0; i < m_dims; i ++)
                                {
                                        u += util::square(x(i));
                                        v += 0.5 * i * x(i);
                                }

                                return u + util::square(v) + util::quartic(v);
                        };

                        const auto fn_grad = [=] (const tvector& x, tvector& gx)
                        {
                                tscalar u = 0, v = 0;
                                for (tsize i = 0; i < m_dims; i ++)
                                {
                                        u += util::square(x(i));
                                        v += 0.5 * i * x(i);
                                }

                                gx.resize(m_dims);
                                for (tsize i = 0; i < m_dims; i ++)
                                {
                                        gx(i) = 2 * x(i) + (2 * v + 4 * util::cube(v)) * 0.5 * i;
                                }

                                return u + util::square(v) + util::quartic(v);
                        };

                        return tproblem(fn_size, fn_fval, fn_grad);
                }

                virtual bool is_valid(const tvector& x) const override
                {
                        return -5.0 < x.minCoeff() && x.maxCoeff() < 10.0;
                }

                virtual bool is_minima(const tvector& x, const tscalar epsilon) const override
                {
                        return util::distance(x, tvector::Zero(m_dims)) < epsilon;
                }

                tsize   m_dims;
        };  
}