#ifndef NANOCV_MODEL_H
#define NANOCV_MODEL_H

#include "ncv_task.h"
#include "ncv_loss.h"

namespace ncv
{
        // manage models (register new ones, query and clone them)
        class model_t;
        typedef manager_t<model_t>              model_manager_t;
        typedef model_manager_t::robject_t      rmodel_t;

        /////////////////////////////////////////////////////////////////////////////////////////
        // generic model: output = model(input).
        /////////////////////////////////////////////////////////////////////////////////////////
                
        class model_t : public clonable_t<model_t>
        {
        public:

                // constructor
                model_t(const string_t& name, const string_t& description)
                        :       clonable_t<model_t>(name, description),
                                m_rows(0),
                                m_cols(0),
                                m_outputs(0)
                {
                }

                // destructor
                virtual ~model_t() {}

                // train the model
                bool train(const task_t& task, const fold_t& fold, const loss_t& loss);

                // evaluate the model (compute the average loss value & error)
                void test(const task_t& task, const fold_t& fold, const loss_t& loss,
                        scalar_t& lvalue, scalar_t& lerror) const;

                // compute the model output
                virtual void process(const vector_t& input, vector_t& output) const = 0;
                void process(const image_t& image, coord_t x, coord_t y, vector_t& output) const;

                // save/load from file
                virtual bool save(const string_t& path) const = 0;
                virtual bool load(const string_t& path) = 0;

                // access functions
                size_t n_rows() const { return m_rows; }
                size_t n_cols() const { return m_cols; }
                size_t n_inputs() const { return 3 * n_rows() * n_cols(); }     // RGB!
                size_t n_outputs() const { return m_outputs; }
                virtual size_t n_parameters() const = 0;

        protected:

                // resize to new inputs/outputs
                virtual void resize() = 0;

                // train the model
                virtual bool _train(const task_t& task, const fold_t& fold, const loss_t& loss) = 0;

        private:

                // attributes
                size_t          m_rows, m_cols;         // input patch size
                size_t          m_outputs;              // output size
        };
}

#endif // NANOCV_MODEL_H
