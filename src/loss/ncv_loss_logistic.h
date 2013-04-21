#ifndef NANOCV_LOSS_LOGISTIC_H
#define NANOCV_LOSS_LOGISTIC_H

#include "ncv_loss.h"

namespace ncv
{
        ////////////////////////////////////////////////////////////////////////////////
        // logistic loss.
        ////////////////////////////////////////////////////////////////////////////////
        
        class logistic_loss : public loss
        {
        public:
                
                // compute the error value
                virtual scalar_t error(const vector_t& targets, const vector_t& scores) const
                {
                        return eclass_error(targets, scores);
                }
                
                // compute the loss value & derivatives
                virtual scalar_t value(const vector_t& targets, const vector_t& scores) const;
                virtual scalar_t vgrad(const vector_t& targets, const vector_t& scores, vector_t& grads) const;

                // create an object clone
                virtual rloss clone(const string_t& /*params*/) const
                {
                        return rloss(new logistic_loss(*this));
                }

                // describe the object
                virtual const char* name() const { return "logistic"; }
                virtual const char* desc() const { return "logistic loss"; }
        };
}

#endif // NANOCV_LOSS_LOGISTIC_H
