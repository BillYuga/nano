#pragma once

#include <cassert>

namespace nano
{
        ///
        /// \brief 2D convolution: omat += imat @ kmat
        ///
        template <typename timatrix, typename tkmatrix, typename tsize, typename tomatrix>
        void conv2d_output(const timatrix& imat, const tkmatrix& kmat, const tsize drows, const tsize dcols,
                tomatrix&& omat)
        {
                const tsize orows = omat.rows();
                const tsize ocols = omat.cols();
                const tsize krows = kmat.rows();
                const tsize kcols = kmat.cols();

                assert(orows * drows + krows - 1 <= imat.rows());
                assert(ocols * dcols + kcols - 1 <= imat.cols());

                for (tsize r = 0; r < orows; ++ r)
                {
                        for (tsize c = 0; c < ocols; ++ c)
                        {
                                for (tsize kr = 0; kr < krows; ++ kr)
                                {
                                        for (tsize kc = 0; kc < kcols; ++ kc)
                                        {
                                                omat(r, c) += imat(r * drows + kr, c * dcols + kc) * kmat(kr, kc);
                                        }
                                }
                        }
                }
        }

        ///
        /// \brief 2D convolution: kmat += imat @ omat
        ///
        template <typename timatrix, typename tkmatrix, typename tsize, typename tomatrix>
        void conv2d_gparam(const timatrix& imat, tkmatrix&& kmat, const tsize drows, const tsize dcols,
                const tomatrix& omat)
        {
                const tsize orows = omat.rows();
                const tsize ocols = omat.cols();
                const tsize krows = kmat.rows();
                const tsize kcols = kmat.cols();

                assert(orows * drows + krows - 1 <= imat.rows());
                assert(ocols * dcols + kcols - 1 <= imat.cols());

                for (tsize r = 0; r < orows; ++ r)
                {
                        for (tsize c = 0; c < ocols; ++ c)
                        {
                                for (tsize kr = 0; kr < krows; ++ kr)
                                {
                                        for (tsize kc = 0; kc < kcols; ++ kc)
                                        {
                                                kmat(kr, kc) += omat(r, c) * imat(r * drows + kr, c * dcols + kc);
                                        }
                                }
                        }
                }
        }

        ///
        /// \brief 2D correlation: imat += omat @ kmat
        ///
        template <typename tomatrix, typename tkmatrix, typename tsize, typename timatrix>
        void corr2d(const tomatrix& omat, const tkmatrix& kmat, const tsize drows, const tsize dcols, timatrix&& imat)
        {
                const tsize orows = omat.rows();
                const tsize ocols = omat.cols();
                const tsize krows = kmat.rows();
                const tsize kcols = kmat.cols();

                assert(orows * drows + krows - 1 <= imat.rows());
                assert(ocols * dcols + kcols - 1 <= imat.cols());

                for (tsize r = 0; r < orows; ++ r)
                {
                        for (tsize c = 0; c < ocols; ++ c)
                        {
                                for (tsize kr = 0; kr < krows; ++ kr)
                                {
                                        for (tsize kc = 0; kc < kcols; ++ kc)
                                        {
                                                imat(r * drows + kr, c * dcols + kc) += omat(r, c) * kmat(kr, kc);
                                        }
                                }
                        }
                }
        }

        ///
        /// \brief
        ///
        template <typename titensor, typename tktensor, typename tvector, typename tsize, typename totensor>
        void conv3d_output(const titensor& idata, const tktensor& kdata, const tvector& bdata,
                const tsize conn, const tsize drows, const tsize dcols, totensor& odata)
        {
                const tsize idims = idata.template size<0>();
                const tsize odims = odata.template size<0>();

                assert(kdata.template size<0>() == odims);
                assert(kdata.template size<1>() == idims / conn);
                assert(bdata.size() == odims);

                for (tsize o = 0; o < odims; ++ o)
                {
                        odata.matrix(o).setConstant(bdata(o));
                        for (tsize i = (o % conn), ik = 0; i < idims; ++ ik, i += conn)
                        {
                                conv2d_output(idata.matrix(i), kdata.matrix(o, ik), drows, dcols, odata.matrix(o));
                        }
                }
        }

        ///
        /// \brief
        ///
        template <typename titensor, typename tktensor, typename tvector, typename tsize, typename totensor>
        void conv3d_ginput(titensor& idata, const tktensor& kdata, const tvector&,
                const tsize conn, const tsize drows, const tsize dcols, const totensor& odata)
        {
                const tsize idims = idata.template size<0>();
                const tsize odims = odata.template size<0>();

                assert(kdata.template size<0>() == odims);
                assert(kdata.template size<1>() == idims / conn);

                idata.setZero();
                for (tsize o = 0; o < odims; ++ o)
                {
                        for (tsize i = (o % conn), ik = 0; i < idims; ++ ik, i += conn)
                        {
                                corr2d(odata.matrix(o), kdata.matrix(o, ik), drows, dcols, idata.matrix(i));
                        }
                }
        }

        ///
        /// \brief
        ///
        template <typename titensor, typename tktensor, typename tvector, typename tsize, typename totensor>
        void conv3d_gparam(const titensor& idata, tktensor&& kdata, tvector&& bdata,
                const tsize conn, const tsize drows, const tsize dcols, const totensor& odata)
        {
                const tsize idims = idata.template size<0>();
                const tsize odims = odata.template size<0>();

                assert(kdata.template size<0>() == odims);
                assert(kdata.template size<1>() == idims / conn);
                assert(bdata.size() == odims);

                kdata.setZero();
                for (tsize o = 0; o < odims; ++ o)
                {
                        bdata(o) = odata.vector(o).sum();
                        for (tsize i = (o % conn), ik = 0; i < idims; ++ ik, i += conn)
                        {
                                conv2d_gparam(idata.matrix(i), kdata.matrix(o, ik), drows, dcols, odata.matrix(o));
                        }
                }
        }
}
