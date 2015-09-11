#pragma once

#include "function.h"

namespace ncv
{
        ///
        /// \brief create Rosenbrock function tests
        ///
        /// https://en.wikipedia.org/wiki/Test_functions_for_optimization
        ///
        NANOCV_PUBLIC std::vector<function_t> make_rosenbrock_funcs(ncv::size_t max_dims = 3);
}
