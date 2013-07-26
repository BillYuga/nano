#include "image.h"
#include "cast.h"
#include <QImage>

namespace ncv
{
        //-------------------------------------------------------------------------------------------------

        bool load_rgba(const string_t& path, rgba_matrix_t& rgba)
        {
                QImage image;
                if (!image.load(path.c_str()))
                {
                        return false;
                }

                const int rows = image.height();
                const int cols = image.width();

                rgba.resize(rows, cols);
                for (int r = 0; r < rows; r ++)
                {
                        for (int c = 0; c < cols; c ++)
                        {
                                const QRgb color = image.pixel(c, r);
                                rgba(r, c) = color::make_rgba(qRed(color),
                                                              qGreen(color),
                                                              qBlue(color),
                                                              qAlpha(color));
                        }
                }

                return true;
        }

        //-------------------------------------------------------------------------------------------------

        bool save_rgba(const string_t& path, const rgba_matrix_t& rgba)
        {
                const int rows = math::cast<int>(rgba.rows());
                const int cols = math::cast<int>(rgba.cols());

                QImage image(cols, rows, QImage::Format_RGB32);

                for (int r = 0; r < rows; r ++)
                {
                        for (int c = 0; c < cols; c ++)
                        {
                                const rgba_t color = rgba(r, c);
                                image.setPixel(c, r, qRgba(
                                        color::make_red(color),
                                        color::make_green(color),
                                        color::make_blue(color),
                                        color::make_alpha(color)));
                        }
                }

                return image.save(path.c_str());
        }

        //-------------------------------------------------------------------------------------------------

        bool image_t::load(const string_t& path)
        {
                return ncv::load_rgba(path, m_rgba);
        }

        //-------------------------------------------------------------------------------------------------

        bool image_t::load_gray(const char* buffer, size_t rows, size_t cols)
        {
                m_rgba.resize(rows, cols);

                for (size_t y = 0, i = 0; y < rows; y ++)
                {
                        for (size_t x = 0; x < cols; x ++, i ++)
                        {
                                m_rgba(y, x) = color::make_rgba(buffer[i], buffer[i], buffer[i]);
                        }
                }

                return true;
        }

        //-------------------------------------------------------------------------------------------------

        bool image_t::load_rgba(const char* buffer, size_t rows, size_t cols)
        {
                m_rgba.resize(rows, cols);

                const size_t size = rows * cols;
                for (size_t y = 0, dr = 0, dg = dr + size, db = dg + size; y < rows; y ++)
                {
                        for (size_t x = 0; x < cols; x ++, dr ++, dg ++, db ++)
                        {
                                m_rgba(y, x) = color::make_rgba(buffer[dr], buffer[dg], buffer[db]);
                        }
                }

                return true;
        }

        //-------------------------------------------------------------------------------------------------

        vector_t image_t::make_target(const rect_t& region) const
        {
                // load the target of the most overlapping annotation
                scalar_t best_overlap = 0.0;
                size_t best_index = 0;

                for (size_t i = 0; i < m_annotations.size(); i ++)
                {
                        const annotation_t& anno = m_annotations[i];

                        const scalar_t overlap = geom::overlap(anno.m_region, region);
                        if (overlap > best_overlap)
                        {
                                best_overlap = overlap;
                                best_index = i;
                        }
                }

                static const scalar_t thres_overlap = 0.80;
                if (    best_overlap > thres_overlap &&
                        best_index < m_annotations.size())
                {
                        return m_annotations[best_index].m_target;
                }
                else
                {
                        vector_t target;
                        return target;
                }
        }

        //-------------------------------------------------------------------------------------------------
}
