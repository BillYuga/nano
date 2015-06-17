#pragma once

#include "corr2d_detail.hpp"
#include <cassert>

namespace ncv
{
        namespace math
        {
                ///
                /// \brief 2D correlation: idata += odata @ kdata (using Eigen 2D blocks)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_egb(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        for (auto r = 0; r < odata.rows(); r ++)
                        {
                                for (auto c = 0; c < odata.cols(); c ++)
                                {
                                        idata.block(r, c, kdata.rows(), kdata.cols()) += kdata * odata(r, c);
                                }
                        }
                }

                ///
                /// \brief 2D correlation: idata += odata @ kdata (using Eigen 1D row-blocks)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_egr(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        for (auto r = 0; r < odata.rows(); r ++)
                        {
                                for (auto kr = 0; kr < kdata.rows(); kr ++)
                                {
                                        for (auto kc = 0; kc < kdata.cols(); kc ++)
                                        {
                                                idata.block(r + kr, kc, 1, odata.cols()) += odata.row(r) * kdata(kr, kc);
                                        }
                                }
                        }
                }

                ///
                /// \brief 2D correlation: idata += odata @ kdata (using plain array indexing)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_cpp(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        detail::corr_cpp(odata, kdata, idata);
                }

                ///
                /// \brief 2D correlation: idata += odata @ kdata (using a mad product by kdata columns)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_mdk(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        detail::corr_madk(odata, kdata, idata);
                }

                ///
                /// \brief 2D correlation: idata += odata @ kdata (using a mad product by odata columns)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_mdo(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        detail::corr_mado(odata, kdata, idata);
                }

                ///
                /// \brief 2D correlation: idata += odata @ kdata (by decoding the kernel size at runtime)
                ///
                template
                <
                        typename tmatrixo,
                        typename tmatrixk = tmatrixo,
                        typename tmatrixi = tmatrixo,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void corr2d_dyn(const tmatrixo& odata, const tmatrixk& kdata, tmatrixi& idata)
                {
                        assert(idata.rows() + 1 == kdata.rows() + odata.rows());
                        assert(idata.cols() + 1 == kdata.cols() + odata.cols());

                        detail::corr_dyn(odata, kdata, idata);
                }
        }
}

