#include "task.h"
#include "common/logger.h"
#include "sampler.h"
#include "grid_image.h"
#include <set>

namespace ncv
{
        rect_t task_t::sample_size() const
        {
                return geom::make_size(n_cols(), n_rows());
        }

        rect_t task_t::sample_region(coord_t x, coord_t y) const
        {
                return geom::make_rect(x, y, n_cols(), n_rows());
        }

        strings_t task_t::labels() const
        {
                // distinct labels
                std::set<string_t> slabels;
                for (const sample_t& sample : m_samples)
                {
                        if (sample.annotated())
                        {
                                slabels.insert(sample.m_label);
                        }
                }

                strings_t result;
                std::copy(slabels.begin(), slabels.end(), std::back_inserter(result));
                return result;
        }

        void task_t::save_as_images(const fold_t& fold, const string_t& basepath, size_t grows, size_t gcols) const
        {
                const size_t border = 8;
                const rgba_t back_color = color::make_rgba(225, 225, 0);

                // process each label ...
                const strings_t labels = this->labels();
                for (size_t l = 0; l <= labels.size(); l ++)    // labels + non-annotated
                {
                        const string_t label = l < labels.size() ? labels[l] : string_t();

                        sampler_t sampler(*this);
                        sampler.setup(fold).setup(label);
                        const samples_t samples = sampler.get();

                        // process all samples with this label ...
                        for (size_t i = 0, g = 1; i < samples.size(); g ++)
                        {
                                grid_image_t grid_image(n_rows(), n_cols(), grows, gcols, border, back_color);

                                // select samples
                                samples_t gsamples;
                                for ( ; i < samples.size() && gsamples.size() < grows * gcols; i ++)
                                {
                                        gsamples.push_back(samples[i]);
                                }

                                // ... compose the image block
                                for (size_t k = 0, r = 0; r < grows; r ++)
                                {
                                        for (size_t c = 0; c < gcols && k < gsamples.size(); c ++, k ++)
                                        {
                                                const sample_t& sample = gsamples[k];
                                                const image_t& image = this->image(sample.m_index);
                                                const rect_t& region = sample.m_region;

                                                grid_image.set(r, c, image.block(
                                                        geom::top(region),
                                                        geom::left(region),
                                                        geom::height(region),
                                                        geom::width(region)));
                                        }
                                }

                                // ... and save it
                                const string_t path = basepath
                                                + (label.empty() ? "" : ("_" + label))
                                                + "_group" + text::to_string(g) + ".png";
                                log_info() << "saving images to <" << path << "> ...";
                                ncv::save_rgba(path, grid_image.rgba());
                        }
                }
        }
}
