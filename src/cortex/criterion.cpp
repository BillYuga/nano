#include "criterion.h"
#include "task.h"
#include "loss.h"
#include <cassert>

namespace cortex
{
        criterion_manager_t& get_criteria()
        {
                static criterion_manager_t manager;
                return manager;
        }

        criterion_t::criterion_t(const string_t& configuration)
                :       clonable_t<criterion_t>(configuration),
                        m_lambda(0.0),
                        m_type(type::value)
        {
        }

        criterion_t& criterion_t::reset(const rmodel_t& rmodel)
        {
                assert(rmodel);
                return reset(*rmodel);
        }

        criterion_t& criterion_t::reset(const model_t& model)
        {
                m_model = model.clone();
                m_model->save_params(m_params);

                return reset();
        }

        criterion_t& criterion_t::reset(const vector_t& params)
        {
                assert(m_model->psize() == params.size());

                m_model->load_params(params);
                m_params = params;

                return reset();
        }

        criterion_t& criterion_t::reset(type t)
        {
                m_type = t;

                return reset();
        }

        criterion_t& criterion_t::reset(scalar_t lambda)
        {
                m_lambda = lambda;

                return reset();
        }

        criterion_t& criterion_t::reset()
        {
                m_estats.clear();

                clear();

                return *this;
        }

        void criterion_t::update(const task_t& task, const sample_t& sample, const loss_t& loss)
        {
                assert(sample.m_index < task.n_images());

                const image_t& image = task.image(sample.m_index);
                const vector_t& target = sample.m_target;
                const vector_t& output = m_model->output(image, sample.m_region).vector();

                assert(output.size() == m_model->osize());
                assert(target.size() == m_model->osize());

                accumulate(output, target, loss);
        }

        void criterion_t::accumulate(const vector_t& output, const vector_t& target, const loss_t& loss)
        {
                const scalar_t value = loss.value(target, output);
                const scalar_t error = loss.error(target, output);

                m_estats(error);

                switch (m_type)
                {
                case type::value:
                        accumulate(value);
                        break;

                case type::vgrad:
                        accumulate(m_model->gparam(loss.vgrad(target, output)), value);
                        break;
                }
        }

        criterion_t& criterion_t::update(const criterion_t& other)
        {
                m_estats(other.m_estats);

                accumulate(other);

                return *this;
        }

        scalar_t criterion_t::avg_error() const
        {
                assert(m_estats.count() > 0);

                return m_estats.avg();
        }

        scalar_t criterion_t::var_error() const
        {
                assert(m_estats.count() > 0);

                return m_estats.var();
        }

        size_t criterion_t::count() const
        {
                return m_estats.count();
        }

        const vector_t& criterion_t::params() const
        {
                return m_params;
        }

        tensor_size_t criterion_t::psize() const
        {
                return m_params.size();
        }

        scalar_t criterion_t::lambda() const
        {
                return m_lambda;
        }
}

