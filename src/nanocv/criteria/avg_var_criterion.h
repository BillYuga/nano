#pragma once

#include "avg_criterion.h"

namespace ncv
{        
        ///
        /// \brief variational regularized loss
        /// (e.g. penalize high variance across training samples),
        ///     like in EBBoost/VadaBoost: http://www.cs.columbia.edu/~jebara/papers/vadaboost.pdf
        ///     or my PhD thesis: http://infoscience.epfl.ch/record/177356?ln=fr
        ///
        class avg_var_criterion_t : public avg_criterion_t
        {
        public:
                
                NANOCV_MAKE_CLONABLE(avg_var_criterion_t, "variational (VadaBoost-like) regularized loss")

                ///
                /// \brief constructor
                ///
                avg_var_criterion_t(const string_t& = string_t());
                
                ///
                /// \brief reset statistics and settings
                ///
                virtual void reset();

                ///
                /// \brief cumulated loss value
                ///
                virtual scalar_t value() const;

                ///
                /// \brief cumulated gradient
                ///
                virtual vector_t vgrad() const;

                ///
                /// \brief check if the criterion has a regularization term to tune
                ///
                virtual bool can_regularize() const;

        protected:

                ///
                /// \brief update statistics with the loss value/error/gradient for a sample
                ///
                virtual void accumulate(scalar_t value, scalar_t error);
                virtual void accumulate(const vector_t& vgrad, scalar_t value, scalar_t error);

                ///
                /// \brief update statistics with cumulated samples
                ///
                virtual void accumulate(const criterion_t& other);
                
        private:
                
                // attributes
                scalar_t        m_value2;        ///< cumulated squared loss value
                vector_t        m_vgrad2;        ///< cumulated loss value multiplied with the gradient
        };
}
