#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test_quadratic"

#include <boost/test/unit_test.hpp>
#include "math/abs.hpp"
#include "math/random.hpp"
#include "math/epsilon.hpp"
#include "min/quadratic.hpp"

BOOST_AUTO_TEST_CASE(test_quadratic)
{
        using namespace ncv;

        const size_t tests = 1327;

        for (size_t t = 0; t < tests; t ++)
        {
                random_t<double> rnd(-1.0, +1.0);

                // build random quadratic
                const double a = rnd();
                const double b = rnd();
                const double c = rnd();
                const quadratic_t<double> q(a, b, c);
                BOOST_CHECK(q);

                const double x0 = rnd();
                const double f0 = q.value(x0);
                const double g0 = q.gradient(x0);

                const double x1 = x0 + rnd() + 1.0;
                const double f1 = q.value(x1);

                // check interpolation
                const quadratic_t<double> iq(x0, f0, g0, x1, f1);
                if (!iq)
                {
                        continue;
                }

                BOOST_CHECK_LE(math::abs(f0 - iq.value(x0)), math::epsilon0<double>());
                BOOST_CHECK_LE(math::abs(g0 - iq.gradient(x0)), math::epsilon0<double>());

                BOOST_CHECK_LE(math::abs(f1 - iq.value(x1)), math::epsilon0<double>());

//                BOOST_CHECK_LE(math::abs(q.a() - iq.a()), math::epsilon1<double>());
//                BOOST_CHECK_LE(math::abs(q.b() - iq.b()), math::epsilon1<double>());
//                BOOST_CHECK_LE(math::abs(q.c() - iq.c()), math::epsilon1<double>());

                // check extremum
                double extremum;
                iq.extremum(extremum);

                if (!std::isfinite(extremum))
                {
                        continue;
                }

                BOOST_CHECK_LE(math::abs(iq.gradient(extremum)), math::epsilon0<double>());

                const size_t etests = 1843;
                for (size_t e = 0; e < etests; e ++)
                {
                        BOOST_CHECK_GE(math::abs(iq.gradient(rnd())),
                                       math::abs(iq.gradient(extremum)));
                }
        }
}
