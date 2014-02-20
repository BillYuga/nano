#ifndef NANOCV_TENSOR_BASE_HPP
#define NANOCV_TENSOR_BASE_HPP

#include <boost/serialization/access.hpp>
#include "matrix.hpp"

namespace ncv
{
        namespace tensor
        {
                ///
                /// store (3D/4D) tensor data using 2D (rows() x cols()) fixed size planes
                ///
                template
                <
                        typename tscalar_,
                        typename tsize_
                >
                class base_t
                {
                public:

                        typedef tscalar_                                        tscalar;
                        typedef tsize_                                          tindex;
                        typedef tsize_                                          tsize;
                        typedef typename vector_types_t<tscalar>::tvector       tvector;
                        typedef typename vector_types_t<tscalar>::tmap          tvector_map;
                        typedef typename matrix_types_t<tscalar>::tmap          tmatrix_map;

                        ///
                        /// \brief constructor
                        /// \param dims total number of fixed size 2D matrices
                        ///
                        base_t(tsize dims = 0, tsize rows = 0, tsize cols = 0)
                        {
                                allocate(dims, rows, cols);
                        }

                        ///
                        /// \brief destructor
                        ///
                        virtual ~base_t()
                        {
                        }

                        ///
                        /// \brief generic tensor access functions
                        ///
                        tsize size() const { return m_data.size(); }
                        tsize dims() const { return m_dims; }
                        tsize rows() const { return m_rows; }
                        tsize cols() const { return m_cols; }

                        const tscalar* data() const { return m_data.data(); }
                        tscalar* data() { return m_data.data(); }

                        tvector_map as_vector() { return tvector_map(data(), size()); }

                protected:

                        ///
                        /// \brief resize to new dimensions
                        ///
                        tsize allocate(tsize dims, tsize rows, tsize cols)
                        {                                
                                m_dims = dims;
                                m_rows = rows;
                                m_cols = cols;
                                m_msize = rows * cols;

                                m_data.resize(m_dims * m_rows * m_cols);
                                m_data.setZero();

                                return size();
                        }

                        ///
                        /// \brief access functions
                        ///
                        const tscalar* matrix_data(tsize i) const { return data() + i * m_msize; }
                        tscalar* matrix_data(tsize i) { return data() + i * m_msize; }

                        tmatrix_map as_matrix(tsize i) { return tmatrix_map(matrix_data(i), rows(), cols()); }

                private:

                        ///
                        /// serialize
                        ///
                        friend class boost::serialization::access;
                        template
                        <
                                class tarchive
                        >
                        void serialize(tarchive & ar, const unsigned int version)
                        {
                                ar & m_dims;
                                ar & m_rows;
                                ar & m_cols;
                                ar & m_msize;
                                ar & m_data;
                        }

                private:

                        // attributes
                        tsize                   m_dims;         ///< #dimensions (depending if 3D or 4D)
                        tsize                   m_rows;         ///< #rows (for each dimension)
                        tsize                   m_cols;         ///< #cols (for each dimension)
                        tsize                   m_msize;        ///< matrix/plane size (rows * cols)
                        tvector                 m_data;         ///< storage (1D vector)
                };
        }
}

#endif // NANOCV_TENSOR_BASE_HPP
