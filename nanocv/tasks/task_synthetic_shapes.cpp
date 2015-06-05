#include "task_synthetic_shapes.h"
#include "nanocv/loss.h"
#include "nanocv/math/numeric.hpp"
#include "nanocv/math/random.hpp"

namespace ncv
{
        synthetic_shapes_task_t::synthetic_shapes_task_t(const string_t& configuration)
                :       task_t(configuration),
                        m_rows(math::clamp(text::from_params<size_t>(configuration, "rows", 32), 16, 128)),
                        m_cols(math::clamp(text::from_params<size_t>(configuration, "cols", 32), 16, 128)),
                        m_outputs(math::clamp(text::from_params<size_t>(configuration, "dims", 4), 2, 10)),
                        m_folds(1),
                        m_color(text::from_params<color_mode>(configuration, "color", color_mode::rgba)),
                        m_size(math::clamp(text::from_params<size_t>(configuration, "size", 1024), 256, 64 * 1024))
        {
        }

        synthetic_shapes_task_t::synthetic_shapes_task_t(
                size_t rows, size_t cols, size_t dims, color_mode color, size_t size)
                :       synthetic_shapes_task_t(
                        "rows=" + text::to_string(rows) + "," +
                        "cols=" + text::to_string(cols) + "," +
                        "dims=" + text::to_string(dims) + "," +
                        "color=" + text::to_string(color) + "," +
                        "size=" + text::to_string(size))
        {
        }

        namespace
        {
                rgba_t make_transparent_color()
                {
                        return 0;
                }

                rect_t make_rect(coord_t rows, coord_t cols)
                {
                        random_t<coord_t> rng(std::min(rows / 8, cols / 8),
                                              std::min(rows / 4, cols / 4));

                        const coord_t dx = rng();
                        const coord_t dy = rng();
                        const coord_t dw = rng();
                        const coord_t dh = rng();

                        return rect_t(dx, dy, cols - dx - dw, rows - dy - dh);
                }

                rect_t make_interior_rect(const rect_t& rect)
                {
                        random_t<coord_t> rngx(1 + (rect.width() + 7) / 8, 1 + (rect.width() + 3) / 4);
                        random_t<coord_t> rngy(1 + (rect.height() + 7) / 8, 1 + (rect.height() + 3) / 4);

                        const coord_t dx = rngx();
                        const coord_t dy = rngy();
                        const coord_t dw = rngx();
                        const coord_t dh = rngy();

                        return rect_t(rect.left() + dx,
                                      rect.top() + dy,
                                      rect.width() - dx - dw,
                                      rect.height() - dy - dh);
                }

                rect_t make_vertical_interior_rect(const rect_t& rect)
                {
                        random_t<coord_t> rng(1 + (rect.width() + 3) / 4, 1 + (rect.width() + 2) / 3);

                        const coord_t dx = rng();
                        const coord_t dy = 0;
                        const coord_t dw = rng();
                        const coord_t dh = 0;

                        return rect_t(rect.left() + dx,
                                      rect.top() + dy,
                                      rect.width() - dx - dw,
                                      rect.height() - dy - dh);
                }

                rect_t make_horizontal_interior_rect(const rect_t& rect)
                {
                        random_t<coord_t> rng(1 + (rect.height() + 3) / 4, 1 + (rect.height() + 2) / 3);

                        const coord_t dx = 0;
                        const coord_t dy = rng();
                        const coord_t dw = 0;
                        const coord_t dh = rng();

                        return rect_t(rect.left() + dx,
                                      rect.top() + dy,
                                      rect.width() - dx - dw,
                                      rect.height() - dy - dh);
                }

                image_t make_filled_rect(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_rectangle(rect, fill_color);

                        return image;
                }

                image_t make_hollow_rect(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_rectangle(rect, fill_color);
                        image.fill_rectangle(make_interior_rect(rect), make_transparent_color());

                        return image;
                }

                image_t make_filled_ellipse(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_ellipse(rect, fill_color);

                        return image;
                }

                image_t make_hollow_ellipse(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_ellipse(rect, fill_color);
                        image.fill_ellipse(make_interior_rect(rect), make_transparent_color());

                        return image;
                }

                image_t make_cross(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill(make_vertical_interior_rect(rect), fill_color);
                        image.fill(make_horizontal_interior_rect(rect), fill_color);

                        return image;
                }

                image_t make_filled_up_triangle(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_up_triangle(rect, fill_color);

                        return image;
                }

                image_t make_hollow_up_triangle(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_up_triangle(rect, fill_color);
                        image.fill_up_triangle(make_interior_rect(rect), make_transparent_color());

                        return image;
                }

                image_t make_filled_down_triangle(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_down_triangle(rect, fill_color);

                        return image;
                }

                image_t make_hollow_down_triangle(coord_t rows, coord_t cols, rgba_t fill_color)
                {
                        const rect_t rect = make_rect(rows, cols);

                        image_t image(rows, cols, color_mode::rgba);

                        image.fill(make_transparent_color());
                        image.fill_down_triangle(rect, fill_color);
                        image.fill_down_triangle(make_interior_rect(rect), make_transparent_color());

                        return image;
                }
        }

        bool synthetic_shapes_task_t::load(const string_t &)
        {
                random_t<size_t> rng_protocol(1, 10);
                random_t<size_t> rng_output(1, osize());

                random_t<scalar_t> rng_gauss(scalar_t(1.0), scalar_t(3.0));

                const coord_t rows = static_cast<coord_t>(irows());
                const coord_t cols = static_cast<coord_t>(icols());

                clear_memory(0);

                for (size_t f = 0; f < fsize(); f ++)
                {
                        for (size_t i = 0; i < m_size; i ++)
                        {
                                // random protocol: train vs. test
                                const protocol p = (rng_protocol() < 9) ? protocol::train : protocol::test;

                                // random output class: #dots
                                const size_t o = rng_output();

                                const rgba_t back_color = color::make_random_rgba();
                                const rgba_t shape_color = color::make_opposite_random_rgba(back_color);

                                // generate random image background
                                image_t image(irows(), icols(), color());
                                image.fill(back_color);
                                image.random_noise(color_channel::rgba, -80.0, +80.0, rng_gauss());

                                // generate random shapes
                                image_t shape;

                                switch (o)
                                {                                
                                case 1:         shape = image; break;
                                case 2:         shape = make_filled_rect(rows, cols, shape_color); break;
                                case 3:         shape = make_hollow_rect(rows, cols, shape_color); break;
                                case 4:         shape = make_filled_ellipse(rows, cols, shape_color); break;
                                case 5:         shape = make_hollow_ellipse(rows, cols, shape_color); break;
                                case 6:         shape = make_cross(rows, cols, shape_color); break;
                                case 7:         shape = make_filled_up_triangle(rows, cols, shape_color); break;
                                case 8:         shape = make_hollow_up_triangle(rows, cols, shape_color); break;
                                case 9:         shape = make_filled_down_triangle(rows, cols, shape_color); break;
                                case 10:        shape = make_hollow_down_triangle(rows, cols, shape_color); break;
                                default:        break;
                                }

                                shape.random_noise(color_channel::rgba, -40.0, +40.0, rng_gauss() * 0.5);
                                image.alpha_blend(shape.rgba());

                                add_image(image);

                                // generate sample
                                sample_t sample(n_images() - 1, sample_region(0, 0));
                                switch (o)
                                {
                                case 1:         sample.m_label = "background"; break;
                                case 2:         sample.m_label = "filled_rectangle"; break;
                                case 3:         sample.m_label = "hollow_rectangle"; break;
                                case 4:         sample.m_label = "filled_ellipse"; break;
                                case 5:         sample.m_label = "hollow_ellipse"; break;
                                case 6:         sample.m_label = "cross"; break;
                                case 7:         sample.m_label = "filled_up_triangle"; break;
                                case 8:         sample.m_label = "hollow_up_triangle"; break;
                                case 9:         sample.m_label = "filled_down_triangle"; break;
                                case 10:        sample.m_label = "hollow_down_triangle"; break;
                                default:        sample.m_label = "unkown"; break;
                                }

                                sample.m_target = ncv::class_target(o - 1, osize());
                                sample.m_fold = {f, p};
                                add_sample(sample);
                        }
                }

                return true;
        }
}
