#include "algorithm.h"
#include <cctype>
#include <algorithm>

namespace nano
{
        string_t lower(const string_t& str)
        {
                string_t ret = str;
                std::transform(str.begin(), str.end(), ret.begin(), [] (char c) { return std::tolower(c); });
                return ret;
        }

        string_t upper(const string_t& str)
        {
                string_t ret = str;
                std::transform(str.begin(), str.end(), ret.begin(), [] (char c) { return std::toupper(c); });
                return ret;
        }

        string_t replace(const string_t& str, const char token, const char newtoken)
        {
                string_t ret = str;
                std::transform(str.begin(), str.end(), ret.begin(),
                               [=] (char c) { return (c == token) ? newtoken : c; });
                return ret;
        }

        string_t replace(const string_t& str, const string_t& token, const string_t& newtoken)
        {
                string_t ret = str;
                size_t index = 0;
                while (true)
                {
                        index = ret.find(token, index);
                        if (index == string_t::npos)
                        {
                                break;
                        }
                        ret.replace(index, token.size(), newtoken);
                        index += newtoken.size();
                }
                return ret;
        }

        bool contains(const string_t& str, const char token)
        {
                return std::find(str.begin(), str.end(), token) != str.end();
        }

        bool equals(const string_t& str1, const string_t& str2)
        {
                return  str1.size() == str2.size() &&
                        std::equal(str1.begin(), str1.end(), str2.begin(),
                                   [] (char c1, char c2) { return c1 == c2; });
        }

        bool iequals(const string_t& str1, const string_t& str2)
        {
                return  str1.size() == str2.size() &&
                        std::equal(str1.begin(), str1.end(), str2.begin(),
                                   [] (char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
        }

        bool starts_with(const string_t& str, const string_t& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.begin(), token.end(), str.begin(),
                                   [] (char c1, char c2) { return c1 == c2; });
        }

        bool istarts_with(const string_t& str, const string_t& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.begin(), token.end(), str.begin(),
                                   [] (char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
        }

        bool ends_with(const string_t& str, const string_t& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.rbegin(), token.rend(), str.rbegin(),
                                   [] (char c1, char c2) { return c1 == c2; });
        }

        bool iends_with(const string_t& str, const string_t& token)
        {
                return  str.size() >= token.size() &&
                        std::equal(token.rbegin(), token.rend(), str.rbegin(),
                                   [] (char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
        }

        strings_t split(const string_t& str, const char* delimeters)
        {
                strings_t tokens;

                // find the beginning of the splitted strings ...
                auto pos_beg = str.find_first_not_of(delimeters);
                while (pos_beg != string_t::npos)
                {
                        // find the end of the splitted strings ...
                        auto pos_end = str.find_first_of(delimeters, pos_beg + 1);
                        if (pos_end == string_t::npos)
                                pos_end = str.size();
                        if (pos_end != pos_beg)
                                tokens.emplace_back(str.substr(pos_beg, pos_end - pos_beg));

                        // continue to iterate for the next splitted string
                        pos_beg = str.find_first_not_of(delimeters, pos_end);
                }

                if (tokens.empty())
                {
                        tokens.push_back(str);
                }

                return tokens;
        }
}
