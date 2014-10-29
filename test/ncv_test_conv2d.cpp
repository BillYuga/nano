#include "nanocv.h"
#include "common/cast.hpp"
#include "common/conv2d.hpp"
#include "common/corr2d.hpp"
#ifdef NANOCV_HAVE_OPENCL
#include "opencl/opencl.h"
#endif
#ifdef NANOCV_HAVE_CUDA
#include "cuda/cuda.h"
#include "cuda/conv2d.h"
#endif
#include <boost/format.hpp>

using namespace ncv;

ncv::thread_pool_t pool;
const size_t tests = 16;

typedef double test_scalar_t;
typedef ncv::tensor::matrix_types_t<test_scalar_t>::tmatrix     test_matrix_t;
typedef ncv::tensor::matrix_types_t<test_scalar_t>::tmatrices   test_matrices_t;

template
<
        typename tmatrix
>
void init_matrix(int rows, int cols, tmatrix& matrix)
{
        matrix.resize(rows, cols);
        matrix.setRandom();
        matrix /= rows;
}

template
<
        typename tmatrix
>
void init_matrices(int rows, int cols, int count, std::vector<tmatrix>& matrices)
{
	matrices.resize(count);
	for (int i = 0; i < count; i ++)
	{
		init_matrix(rows, cols, matrices[i]);
	}
}

template
<
        typename tmatrix
>
void zero_matrices(std::vector<tmatrix>& matrices)
{
        for (size_t i = 0; i < matrices.size(); i ++)
        {
                matrices[i].setZero();
        }
}

template
<
        typename tmatrix,
        typename tscalar = typename tmatrix::Scalar
>
tscalar sum_matrices(std::vector<tmatrix>& matrices)
{
        tscalar sum = 0;
        for (size_t i = 0; i < matrices.size(); i ++)
        {
                sum += matrices[i].sum();
        }
        return sum;
}

template
<
        typename top,
        typename tmatrix,
        typename tscalar = typename tmatrix::Scalar
>
tscalar test_cpu(
        top op, const char* name,
        const std::vector<tmatrix>& idatas, const tmatrix& kdata, std::vector<tmatrix>& odatas)
{
        ncv::stats_t<double, size_t> proc1;
        ncv::stats_t<double, size_t> procx;
        
        // run multiple tests (single threaded)
        for (size_t t = 0; t < tests; t ++)
        {
                zero_matrices(odatas);
                
                const ncv::timer_t timer;
                for (size_t i = 0; i < idatas.size(); i ++)
                {
                        op(idatas[i], kdata, odatas[i]);
                }
                
                proc1(timer.miliseconds());
        }

        const tscalar ret1 = sum_matrices(odatas);

        // run multiple tests (multi threaded)
        for (size_t t = 0; t < tests; t ++)
        {
                zero_matrices(odatas);

                const ncv::timer_t timer;
                ncv::thread_loopi(idatas.size(), pool, [&] (size_t i)
                {
                        op(idatas[i], kdata, odatas[i]);
                });

                procx(timer.miliseconds());
        }

        const tscalar retx = sum_matrices(odatas);

        const string_t time_str = (boost::format("%1%/%2%") %
                math::cast<size_t>(proc1.min()) %
                math::cast<size_t>(procx.min())).str();
        
        std::cout << name << "= " << text::resize(time_str, 6, align::right) << "ms     ";

        if (ret1 != retx)
        {
                throw std::runtime_error(string_t("missmatch between single & multi-threaded version of ") + name);
        }
        
        return ret1;
}

#ifdef NANOCV_HAVE_OPENCL

const std::string conv_program_source = R"xxx(

#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void conv_kernel(
        __global const double* idata,
        __constant double* kdata, int krows, int kcols,
        __global double* odata)
{        
        const int c = get_global_id(0);
        const int r = get_global_id(1);

        const int ocols = get_global_size(0);
        const int orows = get_global_size(1);

        const int icols = ocols + kcols - 1;

        double sum = 0;
        for (int kr = 0; kr < krows; kr ++)
        {
                for (int kc = 0; kc < kcols; kc ++)
                {
                        sum += idata[(r + kr) * icols + (c + kc)] * kdata[kr * kcols + kc];
                }
        }

        odata[r * ocols + c] = sum;
}

__kernel void corr_kernel(
        __global const double* odata,
        __constant double* kdata, int krows, int kcols,
        __global double* idata)
{
        const int c = get_global_id(0);
        const int r = get_global_id(1);

        const int icols = get_global_size(0);
        const int irows = get_global_size(1);

        const int orows = irows - krows + 1;
        const int ocols = icols - kcols + 1;

        const int krmin = max(0,     r - orows + 1);
        const int krmax = min(krows, r + 1);

        const int kcmin = max(0,     c - ocols + 1);
        const int kcmax = min(kcols, c + 1);

        double sum = 0;
        for (int kr = krmin; kr < krmax; kr ++)
        {
                for (int kc = kcmin; kc < kcmax; kc ++)
                {
                        sum += odata[(r - kr) * ocols + (c - kc)] * kdata[kr * kcols + kc];
                }
        }

        idata[r * icols + c] = sum;
}

)xxx";

template
<
        typename tmatrix,
        typename tscalar = typename tmatrix::Scalar
>
tscalar test_gpu(
        const char* kernel_name, const char* name,
        const std::vector<tmatrix>& idatas, const tmatrix& kdata, std::vector<tmatrix>& odatas)
{
        ocl::manager_t& theocl = ocl::manager_t::instance();

        const cl::Context context = theocl.make_context();
        const cl::CommandQueue queue = theocl.make_command_queue(context);
        const cl::Program program = theocl.make_program_from_text(context, conv_program_source);
        cl::Kernel kernel = theocl.make_kernel(program, kernel_name);

        const int krows = static_cast<int>(kdata.rows());
        const int kcols = static_cast<int>(kdata.cols());
        const int orows = static_cast<int>(odatas[0].rows());
        const int ocols = static_cast<int>(odatas[0].cols());

        const size_t mem_idata = idatas[0].size() * sizeof(tscalar);
        const size_t mem_kdata = kdata.size() * sizeof(tscalar);
        const size_t mem_odata = odatas[0].size() * sizeof(tscalar);

        // create buffers once
        const cl::Buffer ibuffer = theocl.make_buffer(context, mem_idata, CL_MEM_READ_ONLY);
        const cl::Buffer kbuffer = theocl.make_buffer(context, mem_kdata, CL_MEM_READ_ONLY);
        const cl::Buffer obuffer = theocl.make_buffer(context, mem_odata, CL_MEM_WRITE_ONLY);

        // setup kernel buffers once
        kernel.setArg(0, ibuffer);
        kernel.setArg(1, kbuffer);
        kernel.setArg(2, sizeof(int), (void*)&krows);
        kernel.setArg(3, sizeof(int), (void*)&kcols);
        kernel.setArg(4, obuffer);

        // transfer constants
        queue.enqueueWriteBuffer(kbuffer, CL_TRUE, 0, mem_kdata, kdata.data());

        ncv::stats_t<double, size_t> proc_stats;

        // run multiple tests
        for (size_t t = 0; t < tests; t ++)
        {
                zero_matrices(odatas);

                const ncv::timer_t timer;
                for (size_t i = 0; i < idatas.size(); i ++)
                {
                        queue.enqueueWriteBuffer(ibuffer, CL_TRUE, 0, mem_idata, idatas[i].data());

                        queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                                                   cl::NDRange(ocols, orows),
                                                   cl::NDRange(ocols, orows));
                        queue.finish();

                        queue.enqueueReadBuffer(obuffer, CL_TRUE, 0, mem_odata, odatas[i].data());
                }

                proc_stats(timer.miliseconds());
        }

        const size_t milis = static_cast<size_t>(proc_stats.min());
        std::cout << name << "= " << text::resize(text::to_string(milis), 4, align::right) << "ms  ";

        return sum_matrices(odatas);
}

#endif

#ifdef NANOCV_HAVE_CUDA

template
<
        typename top,        
        typename tmatrix,        
        typename tscalar = typename tmatrix::Scalar
>
tscalar test_gpu(
        top op, const char* name, 
        const std::vector<tmatrix>& idatas, const tmatrix& kdata, std::vector<tmatrix>& odatas)
{
        const int irows = static_cast<int>(idatas[0].rows());
        const int icols = static_cast<int>(idatas[0].cols());
        const int krows = static_cast<int>(kdata.rows());
        const int kcols = static_cast<int>(kdata.cols());
        const int orows = static_cast<int>(odatas[0].rows());
        const int ocols = static_cast<int>(odatas[0].cols());

        cuda::matrix_t<tscalar> d_idata(irows, icols);
        cuda::matrix_t<tscalar> d_kdata(krows, kcols);
        cuda::matrix_t<tscalar> d_odata(orows, ocols);

        ncv::stats_t<double, size_t> proc_stats;

        // run multiple tests
        for (size_t t = 0; t < tests; t ++)
        {
                zero_matrices(odatas);

                const ncv::timer_t timer;

                d_kdata.to_device(kdata.data());

                for (size_t i = 0; i < idatas.size(); i ++)
                {
                        d_idata.to_device(idatas[i].data());

                        op(d_idata, d_kdata, d_odata, 0);

                        d_odata.from_device(odatas[i].data());
                }

                proc_stats(timer.miliseconds());
        }

        const size_t milis = static_cast<size_t>(proc_stats.min());
        std::cout << name << "= " << text::resize(text::to_string(milis), 4, align::right) << "ms  ";

        return sum_matrices(odatas);
}

#endif

template
<
        typename tscalar
>
void check(tscalar result, tscalar baseline, const char* name)
{
        const tscalar err = math::abs(result - baseline);
        if (!math::almost_equal(err, tscalar(0)))
        {
                std::cout << name << " FAILED (diff = " << err << ")!" << std::endl;
        }
}

void test_conv2d(int isize, int ksize, int tsize)
{
        const int osize = isize - ksize + 1;

        test_matrices_t idatas, odatas;
        test_matrix_t kdata;

        init_matrices(isize, isize, tsize, idatas);
        init_matrices(osize, osize, tsize, odatas);
        init_matrix(ksize, ksize, kdata);
        
        const string_t header = (boost::format("%5% x (%1%x%2%@%3%x%4%): ") % isize % isize % ksize % ksize % tsize).str();
        std::cout << text::resize(header, 24);
        
        const test_scalar_t convcpu_eig = test_cpu(ncv::conv2d_eig<test_matrix_t>, "conv-eig", idatas, kdata, odatas);
        const test_scalar_t convcpu_cpp = test_cpu(ncv::conv2d_cpp<test_matrix_t>, "conv-cpp", idatas, kdata, odatas);
        const test_scalar_t convcpu_dot = test_cpu(ncv::conv2d_dot<test_matrix_t>, "conv-dot", idatas, kdata, odatas);
        const test_scalar_t convcpu_mad = test_cpu(ncv::conv2d_mad<test_matrix_t>, "conv-mad", idatas, kdata, odatas);
        const test_scalar_t convcpu_dyn = test_cpu(ncv::conv2d_dyn<test_matrix_t>, "conv-dyn", idatas, kdata, odatas);
#if defined(NANOCV_HAVE_OPENCL)
        const test_scalar_t convgpu    = test_gpu("conv_kernel", "conv-gpu", idatas, kdata, odatas);
#elif defined(NANOCV_HAVE_CUDA)
        const test_scalar_t convgpu    = test_gpu(cuda::conv2d<test_scalar_t>, "conv-gpu", idatas, kdata, odatas);
#endif
        std::cout << std::endl;

        check(convcpu_eig, convcpu_eig, "conv-eig");
        check(convcpu_cpp, convcpu_eig, "conv-cpp");
        check(convcpu_dot, convcpu_eig, "conv-dot");
        check(convcpu_mad, convcpu_eig, "conv-mad");
        check(convcpu_dyn, convcpu_eig, "conv-dyn");
#if defined(NANOCV_HAVE_OPENCL) || defined(NANOCV_HAVE_CUDA)
        check(convgpu    , convcpu_eig, "conv-gpu");
#endif
}

void test_corr2d(int isize, int ksize, int tsize)
{
        const int osize = isize - ksize + 1;

        test_matrices_t idatas, odatas;
        test_matrix_t kdata;

        init_matrices(isize, isize, tsize, idatas);
        init_matrices(osize, osize, tsize, odatas);
        init_matrix(ksize, ksize, kdata);

        const string_t header = (boost::format("%5% x (%1%x%2%@%3%x%4%): ") % isize % isize % ksize % ksize % tsize).str();
        std::cout << text::resize(header, 24);

        const test_scalar_t corrcpu_eig = test_cpu(ncv::corr2d_eig<test_matrix_t>, "corr-eig", odatas, kdata, idatas);
        const test_scalar_t corrcpu_cpp = test_cpu(ncv::corr2d_cpp<test_matrix_t>, "corr-cpp", odatas, kdata, idatas);
        const test_scalar_t corrcpu_mad = test_cpu(ncv::corr2d_mad<test_matrix_t>, "corr-mad", odatas, kdata, idatas);
        const test_scalar_t corrcpu_dyn = test_cpu(ncv::corr2d_dyn<test_matrix_t>, "corr-dyn", odatas, kdata, idatas);
#if defined(NANOCV_HAVE_OPENCL)
        const test_scalar_t corrgpu   = test_gpu("corr_kernel", "corr-gpu", odatas, kdata, idatas);
#elif NANOCV_HAVE_CUDA
        const test_scalar_t corrgpu   = test_gpu(cuda::corr2d<test_scalar_t>, "corr-gpu", odatas, kdata, idatas);
#endif
        std::cout << std::endl;

        check(corrcpu_eig, corrcpu_eig, "corr-eig");
        check(corrcpu_cpp, corrcpu_eig, "corr-cpp");
        check(corrcpu_mad, corrcpu_eig, "corr-mad");
        check(corrcpu_dyn, corrcpu_eig, "corr-dyn");
#if defined(NANOCV_HAVE_OPENCL) || defined(NANOCV_HAVE_CUDA)
        check(corrgpu    , corrcpu_eig, "corr-gpu");
#endif
}

int main(int argc, char* argv[])
{
#ifdef NANOCV_HAVE_OPENCL
        if (!ocl::manager_t::instance().valid())
        {
                exit(EXIT_FAILURE);
        }
#endif

#ifdef NANOCV_HAVE_CUDA
        cuda::print_info();
#endif

        static const int min_isize = 24;
        static const int max_isize = 48;
        static const int min_ksize = 5;
        static const int max_n_samples = 4000;
        static const int var_samples = 500;

#ifdef NANOCV_HAVE_OPENCL
        try
#endif
        {
                for (int isize = min_isize, n_samples = max_n_samples; isize <= max_isize; isize += 4, n_samples -= var_samples)
                {
                        for (int ksize = min_ksize; ksize <= isize - min_ksize; ksize += 2)
                        {
                                test_conv2d(isize, ksize, n_samples);
                        }
                        std::cout << std::endl;

                        for (int ksize = min_ksize; ksize <= isize - min_ksize; ksize += 2)
                        {
                                test_corr2d(isize, ksize, n_samples);
                        }
                        std::cout << std::endl;
                }
        }

#ifdef NANOCV_HAVE_OPENCL
        catch (cl::Error& e)
        {
                log_error() << "OpenCL fatal error: <" << e.what() << "> (" << ocl::error_string(e.err()) << ")!";
        }
#endif

	return EXIT_SUCCESS;
}

