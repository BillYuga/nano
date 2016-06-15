#pragma once

#include "to_string.hpp"

namespace nano
{
        ///
        /// \brief a row in the table.
        ///
        class table_row_t
        {
        public:

                ///
                /// \brief constructor
                ///
                explicit table_row_t(const string_t& name) :
                        m_name(name)
                {
                }

                ///
                /// \brief append a column value
                ///
                template
                <
                        typename tvalue
                >
                table_row_t& operator<<(tvalue value)
                {
                        m_values.emplace_back(nano::to_string(value));
                        m_markings.push_back(string_t());
                        return *this;
                }

                ///
                /// \brief retrieve the row name
                ///
                const auto& name() const { return m_name; }

                ///
                /// \brief retrieve the column values & markings
                ///
                const auto& value(const size_t i) const { return m_values.at(i); }
                auto& value(const size_t i) { return m_values.at(i); }

                const auto& marking(const size_t i) const { return m_markings.at(i); }
                auto& marking(const size_t i) { return m_markings.at(i); }

                ///
                /// \brief retrieve the column value range
                ///
                auto begin() const { return m_values.cbegin(); }
                auto end() const { return m_values.cend(); }

                ///
                /// \brief retrieve the number of columns
                ///
                size_t size() const { return m_values.size(); }

        private:

                // attributes
                string_t                m_name;         ///< row name
                strings_t               m_values;       ///< column values
                strings_t               m_markings;     ///< column marking (e.g. min|max decoration)
        };
}

