#include "loss.h"

namespace ncv
{
        /////////////////////////////////////////////////////////////////////////////////////////

        vector_t class_target(size_t ilabel, size_t n_labels)
        {
                vector_t target(n_labels);
                target.setConstant(neg_target());
                if (ilabel < n_labels)
                {
                        target[ilabel] = pos_target();
                }
                return target;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        scalar_t l1_error(const vector_t& targets, const vector_t& scores)
        {
                return (targets - scores).array().abs().sum();
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        scalar_t sign_class_error(const vector_t& targets, const vector_t& scores)
        {
                const scalar_t eps = std::numeric_limits<scalar_t>::epsilon();
                return ((targets.array() * scores.array()) < eps).count() > 0;
        }

        /////////////////////////////////////////////////////////////////////////////////////////

        scalar_t multi_class_error(const vector_t& targets, const vector_t& scores)
        {
                std::ptrdiff_t idx = 0;
                scores.maxCoeff(&idx);

                return targets(idx) > 0.5 ? 0.0 : 1.0;
        }

        /////////////////////////////////////////////////////////////////////////////////////////
}
	
