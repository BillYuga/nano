#pragma once

#include "dot.hpp"
#include "mad.hpp"

namespace ncv
{
        namespace detail
        {
                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_cpp(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();
                        const auto krows = kdata.rows();
                        const auto kcols = kdata.cols();
                        const auto icols = idata.cols();

                        for (auto r = 0; r < orows; r ++)
                        {
                                tscalar* podata = odata.data() + r * ocols;

                                for (auto kr = 0; kr < krows; kr ++)
                                {
                                        const tscalar* pidata = idata.data() + (r + kr) * icols;
                                        const tscalar* pkdata = kdata.data() + kr * kcols;

                                        for (auto c = 0; c < ocols; c ++)
                                        {
                                                tscalar sum = 0;
                                                for (auto kc = 0; kc < kcols; kc ++)
                                                {
                                                        sum += pidata[c + kc] * pkdata[kc];
                                                }
                                                podata[c] += sum;
                                        }
                                }
                        }
                }

                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tdotop,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_dot(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata, const tdotop& dotop)
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();
                        const auto krows = kdata.rows();
                        const auto kcols = kdata.cols();
                        const auto icols = idata.cols();

                        for (auto r = 0; r < orows; r ++)
                        {
                                tscalar* podata = odata.data() + r * ocols;

                                for (auto kr = 0; kr < krows; kr ++)
                                {
                                        const tscalar* pidata = idata.data() + (r + kr) * icols;
                                        const tscalar* pkdata = kdata.data() + kr * kcols;

                                        for (auto c = 0; c < ocols; c ++)
                                        {
                                                podata[c] += dotop(pidata + c, pkdata, kcols);
                                        }
                                }
                        }
                }

                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tmadop,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_mad(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata, const tmadop& madop)
                {
                        const auto orows = odata.rows();
                        const auto ocols = odata.cols();
                        const auto krows = kdata.rows();
                        const auto kcols = kdata.cols();
                        const auto icols = idata.cols();

                        for (auto r = 0; r < orows; r ++)
                        {
                                tscalar* podata = odata.data() + r * ocols;

                                for (auto kr = 0; kr < krows; kr ++)
                                {
                                        const tscalar* pidata = idata.data() + (r + kr) * icols;
                                        const tscalar* pkdata = kdata.data() + kr * kcols;

                                        for (auto kc = 0; kc < kcols; kc ++)
                                        {
                                                madop(pidata + kc, pkdata[kc], ocols, podata);
                                        }
                                }
                        }
                }

                template
                <
                        typename tmatrixi,
                        typename tmatrixk = tmatrixi,
                        typename tmatrixo = tmatrixi,
                        typename tscalar = typename tmatrixi::Scalar
                >
                void conv_dyn(const tmatrixi& idata, const tmatrixk& kdata, tmatrixo& odata)
                {
                        const auto kcols = kdata.cols();
                        const auto ocols = odata.cols();                        

                        // decode at run-time the kernel size
                        if (kcols < ocols)
                        {
                                switch (kcols)
                                {
                                case 1:         conv_dot(idata, kdata, odata, math::dot<tscalar, 1>); break;
                                case 2:         conv_dot(idata, kdata, odata, math::dot<tscalar, 2>); break;
                                case 3:         conv_dot(idata, kdata, odata, math::dot<tscalar, 3>); break;
                                case 4:         conv_dot(idata, kdata, odata, math::dot<tscalar, 4>); break;
                                case 5:         conv_dot(idata, kdata, odata, math::dot<tscalar, 5>); break;
                                case 6:         conv_dot(idata, kdata, odata, math::dot<tscalar, 6>); break;
                                case 7:         conv_dot(idata, kdata, odata, math::dot<tscalar, 7>); break;
                                case 8:         conv_dot(idata, kdata, odata, math::dot<tscalar, 8>); break;
                                case 9:         conv_dot(idata, kdata, odata, math::dot<tscalar, 9>); break;
                                case 10:        conv_dot(idata, kdata, odata, math::dot<tscalar, 10>); break;
                                case 11:        conv_dot(idata, kdata, odata, math::dot<tscalar, 11>); break;
                                case 12:        conv_dot(idata, kdata, odata, math::dot<tscalar, 12>); break;
                                case 13:        conv_dot(idata, kdata, odata, math::dot<tscalar, 13>); break;
                                case 14:        conv_dot(idata, kdata, odata, math::dot<tscalar, 14>); break;
                                case 15:        conv_dot(idata, kdata, odata, math::dot<tscalar, 15>); break;
                                default:        conv_dot(idata, kdata, odata, math::dot<tscalar>); break;
                                }
                        }
                        else
                        {
                                switch (ocols)
                                {
                                case 1:         conv_mad(idata, kdata, odata, math::mad<tscalar, 1>); break;
                                case 2:         conv_mad(idata, kdata, odata, math::mad<tscalar, 2>); break;
                                case 3:         conv_mad(idata, kdata, odata, math::mad<tscalar, 3>); break;
                                case 4:         conv_mad(idata, kdata, odata, math::mad<tscalar, 4>); break;
                                case 5:         conv_mad(idata, kdata, odata, math::mad<tscalar, 5>); break;
                                case 6:         conv_mad(idata, kdata, odata, math::mad<tscalar, 6>); break;
                                case 7:         conv_mad(idata, kdata, odata, math::mad<tscalar, 7>); break;
                                case 8:         conv_mad(idata, kdata, odata, math::mad<tscalar, 8>); break;
                                case 9:         conv_mad(idata, kdata, odata, math::mad<tscalar, 9>); break;
                                case 10:        conv_mad(idata, kdata, odata, math::mad<tscalar, 10>); break;
                                case 11:        conv_mad(idata, kdata, odata, math::mad<tscalar, 11>); break;
                                case 12:        conv_mad(idata, kdata, odata, math::mad<tscalar, 12>); break;
                                case 13:        conv_mad(idata, kdata, odata, math::mad<tscalar, 13>); break;
                                case 14:        conv_mad(idata, kdata, odata, math::mad<tscalar, 14>); break;
                                case 15:        conv_mad(idata, kdata, odata, math::mad<tscalar, 15>); break;
                                default:        conv_mad(idata, kdata, odata, math::mad<tscalar>); break;
                                }
                        }
                }
        }
}

