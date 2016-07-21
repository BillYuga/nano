#pragma once

#include <cuda.h>
#include <cuda_runtime.h>

namespace nano
{
        namespace cuda
        {
                ///
                /// \brief allocated 1D buffer on the device.
                ///
                class stream_t
                {
                public:

                        ///
                        /// \brief constructor
                        ///
                        stream_t()
                                :       m_stream(NULL)
                        {
                        }

                        ///
                        /// \brief disable copying
                        ///
                        stream_t(const stream_t&);
                        stream_t& operator=(const stream_t&);

                        ///
                        /// \brief destructor
                        ///
                        ~stream_t()
                        {
                                cudaStreamDestroy(m_stream);
                        }

                        ///
                        /// \brief create a stream
                        ///
                        cudaError make()
                        {
                                cudaStreamDestroy(m_stream);
                                m_stream = NULL;

                                return cudaStreamCreate(&m_stream);
                        }

                        ///
                        /// \brief synchronize the associated stream
                        ///
                        cudaError synchronize() const
                        {
                                return valid() ? cudaStreamSynchronize(m_stream) : cudaErrorNotReady;
                        }

                        ///
                        /// \brief access functions
                        ///
                        bool valid() const { return m_stream != NULL; }
                        cudaStream_t data() const { return m_stream; }

                protected:

                        // attributes
                        cudaStream_t            m_stream;
                };
        }
}

