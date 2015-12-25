#include "layer_plane_affine.h"
#include "math/clamp.hpp"
#include "math/random.hpp"
#include "tensor/random.hpp"
#include "text/from_params.hpp"
#include "tensor/serialize.hpp"

namespace cortex
{
        plane_affine_layer_t::plane_affine_layer_t(const string_t& parameters)
                :       layer_t(parameters)
        {
        }

        tensor_size_t plane_affine_layer_t::resize(const tensor_t& tensor)
        {
                const auto idims = tensor.dims();
                const auto odims = math::clamp(text::from_params<tensor_size_t>(configuration(), "dims", 10), 1, 4096);

                // resize buffers
                m_idata.resize(idims, tensor.rows(), tensor.cols());
                m_odata.resize(odims, tensor.rows(), tensor.cols());

                m_wdata.resize(odims, idims);
                m_bdata.resize(odims);

                return psize();
        }

        void plane_affine_layer_t::zero_params()
        {
                m_wdata.setZero();
                m_bdata.setZero();
        }

        void plane_affine_layer_t::random_params(scalar_t min, scalar_t max)
        {
                tensor::set_random(m_wdata, math::random_t<scalar_t>(min, max));
                tensor::set_random(m_bdata, math::random_t<scalar_t>(min, max));
        }

        scalar_t* plane_affine_layer_t::save_params(scalar_t* params) const
        {
                params = tensor::to_array(m_wdata, params);
                params = tensor::to_array(m_bdata, params);
                return params;
        }

        const scalar_t* plane_affine_layer_t::load_params(const scalar_t* params)
        {
                params = tensor::from_array(m_wdata, params);
                params = tensor::from_array(m_bdata, params);
                return params;
        }

        const tensor_t& plane_affine_layer_t::output(const tensor_t& input)
        {
                assert(idims() == input.dims());
                assert(irows() == input.rows());
                assert(icols() == input.cols());

                m_idata = input;

                for (tensor_size_t o = 0; o < odims(); ++ o)
                {
                        auto odata = m_odata.matrix(o);

                        odata.setConstant(m_bdata(o));
                        for (tensor_size_t i = 0; i < idims(); ++ i)
                        {
                                odata += m_wdata(o, i) * m_idata.matrix(i);
                        }
                }

                return m_odata;
        }

        const tensor_t& plane_affine_layer_t::ginput(const tensor_t& output)
        {
                assert(output.dims() == odims());
                assert(output.rows() == orows());
                assert(output.cols() == ocols());

                m_odata = output;

                for (tensor_size_t i = 0; i < idims(); ++ i)
                {
                        auto idata = m_idata.matrix(i);

                        idata.setZero();
                        for (tensor_size_t o = 0; o < odims(); ++ o)
                        {
                                idata += m_wdata(o, i) * m_odata.matrix(o);
                        }
                }

                return m_idata;
        }

        void plane_affine_layer_t::gparam(const tensor_t& output, scalar_t* gradient)
        {
                assert(output.dims() == odims());
                assert(output.rows() == orows());
                assert(output.cols() == ocols());

                m_odata = output;

                auto gwdata = tensor::map_matrix(gradient, m_wdata.rows(), m_wdata.cols());
                auto gbdata = tensor::map_vector(gradient + m_wdata.size(), m_bdata.rows());

                for (tensor_size_t o = 0; o < odims(); ++ o)
                {
                        for (tensor_size_t i = 0; i < idims(); ++ i)
                        {
                                gwdata(o, i) = (m_odata.matrix(o).array() * m_idata.matrix(i).array()).sum();
                        }
                }

                for (tensor_size_t o = 0; o < odims(); ++ o)
                {
                        gbdata(o) = m_odata.matrix(o).sum();
                }
        }
}

