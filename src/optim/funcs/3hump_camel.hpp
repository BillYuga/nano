#pragma once

#include "util.hpp"
#include "function.hpp"
#include <cmath>

namespace nano
{
        ///
        /// \brief create three-hump camel test functions
        ///
        /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
        ///
        struct function_3hump_camel_t : public function_t
        {
                virtual std::string name() const override
                {
                        return "3hump camel";
                }

                virtual problem_t problem() const override
                {
                        const auto fn_size = [=] ()
                        {
                                return 2;
                        };

                        const auto fn_fval = [=] (const vector_t& x)
                        {
                                const auto a = x(0), b = x(1);

                                const auto a2 = a * a;
                                const auto a4 = a2 * a2;
                                const auto a6 = a4 * a2;

                                return scalar_t(2) * a2 - scalar_t(1.05) * a4 + a6 / scalar_t(6.0) + a * b + b * b;
                        };

                        const auto fn_grad = [=] (const vector_t& x, vector_t& gx)
                        {
                                const auto a = x(0), b = x(1);

                                const auto a2 = a * a;
                                const auto a3 = a * a2;
                                const auto a5 = a3 * a2;

                                gx.resize(2);
                                gx(0) = scalar_t(4) * a - scalar_t(1.05) * scalar_t(4) * a3 + a5 + b;
                                gx(1) = a + scalar_t(2) * b;

                                return fn_fval(x);
                        };

                        return {fn_size, fn_fval, fn_grad};
                }

                virtual bool is_valid(const vector_t& x) const override
                {
                        return util::norm(x) < scalar_t(5.0);
                }

                virtual bool is_minima(const vector_t& x, const scalar_t epsilon) const override
                {
                        const auto a = scalar_t(4.2);
                        const auto b = std::sqrt(scalar_t(3.64));

                        const auto xmp = std::sqrt(scalar_t(0.5) * (a + b));
                        const auto xmn = std::sqrt(scalar_t(0.5) * (a - b));

                        const auto xmins =
                        {
                                std::vector<scalar_t>{ scalar_t(0.0), scalar_t(0.0) },
                                std::vector<scalar_t>{ xmp, scalar_t(-0.5) * xmp },
                                std::vector<scalar_t>{ xmn, scalar_t(-0.5) * xmn },
                                std::vector<scalar_t>{ -xmp, scalar_t(0.5) * xmp },
                                std::vector<scalar_t>{ -xmn, scalar_t(0.5) * xmn }
                        };

                        return util::check_close(x, xmins, epsilon);
                }
        };
}