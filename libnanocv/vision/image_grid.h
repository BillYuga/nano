#pragma once

#include "image.h"

namespace ncv
{
        ///
        /// \brief create an RGBA image composed from fixed-size RGBA patches disposed in a grid
        ///
        class NANOCV_PUBLIC image_grid_t
        {
        public:

                // constructor
                image_grid_t(   size_t patch_rows, size_t patch_cols,
                                size_t group_rows, size_t group_cols,
                                size_t border = 8,
                                rgba_t back_color = color::make_rgba(225, 225, 0));

                // setup a patch at a given grid position
                bool set(size_t grow, size_t gcol, const image_t& image);
                bool set(size_t grow, size_t gcol, const image_t& image, const rect_t& region);

                // access functions
                const image_t& image() const { return m_image; }

        private:

                // attributes
                size_t          m_prows;        ///< patch size
                size_t          m_pcols;
                size_t          m_grows;        ///< grid size
                size_t          m_gcols;
                size_t          m_border;       ///< grid border in pixels
                rgba_t          m_bcolor;       ///< background color
                image_t         m_image;
        };
}

