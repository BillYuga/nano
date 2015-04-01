#pragma once

#include "function.h"

namespace ncv
{
        ///
        /// \brief create Powell function tests
        ///
        std::vector<function_t> make_powell_funcs(ncv::size_t max_dims = 32);
}