#include "nanocv.h"
#include "util/mad.hpp"
#include "util/tabulator.h"
#include "tensor/mad.hpp"

using namespace ncv;

template
<
        typename top,
        typename tvector,
        typename tscalar = typename tvector::Scalar
>
void test_mad(tabulator_t::row_t& row, top op, const tvector& vec1, const tvector& vec2, tscalar wei)
{
        vector_t cvec1 = vec1;
        vector_t cvec2 = vec2;

        const ncv::timer_t timer;

        op(cvec1.data(), wei, cvec1.size(), cvec2.data());

        row << timer.microseconds();
}

void test_mad(size_t size, tabulator_t::row_t& row)
{
        vector_t vec1(size), vec2(size);
        vec1.setRandom();
        vec2.setRandom();

        scalar_t wei = vec1(0) + vec2(3);

        test_mad(row, ncv::mad<scalar_t>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 2>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 3>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 4>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 5>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 6>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 7>, vec1, vec2, wei);
        test_mad(row, ncv::mad_unroll<scalar_t, 8>, vec1, vec2, wei);
        test_mad(row, ncv::tensor::mad_eig<scalar_t>, vec1, vec2, wei);
}

int main(int argc, char* argv[])
{
        static const size_t min_size = 32 * 1024;
        static const size_t max_size = 4 * 1024 * 1024;

        tabulator_t table("size\\mad");

        table.header() << "mad [us]"
                       << "madul2 [us]"
                       << "madul3 [us]"
                       << "madul4 [us]"
                       << "madul5 [us]"
                       << "madul6 [us]"
                       << "madul7 [us]"
                       << "madul8 [us]"
                       << "madeig [us]";

        for (size_t size = min_size; size <= max_size; size *= 2)
        {
                tabulator_t::row_t& row = table.append(text::to_string(size / 1024) + "K");

                test_mad(size, row);
        }

        table.print(std::cout);

	return EXIT_SUCCESS;
}
