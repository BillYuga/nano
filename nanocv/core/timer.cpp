#include "timer.h"
#include <utility>
#include <sstream>
#include <iomanip>

namespace ncv
{
        std::string timer_t::miliseconds_to_string(std::size_t count)
        {
                static const std::size_t size_second = 1000;
                static const std::size_t size_minute = 60 * size_second;
                static const std::size_t size_hour = 60 * size_minute;
                static const std::size_t size_day = 24 * size_hour;

                const std::size_t days = count / size_day; count -= days * size_day;
                const std::size_t hours = count / size_hour; count -= hours * size_hour;
                const std::size_t minutes = count / size_minute; count -= minutes * size_minute;
                const std::size_t seconds = count / size_second; count -= seconds * size_second;
                const std::size_t miliseconds = count;

                std::stringstream stream;
                if (days > 0)
                {
                        stream << days << "d:";
                }
                if (days > 0 || hours > 0)
                {
                        stream << std::setfill('0') << std::setw(2) << hours << "h:";
                }
                if (days > 0 || hours > 0 || minutes > 0)
                {
                        stream << std::setfill('0') << std::setw(2) << minutes << "m:";
                }
                if (days > 0 || hours > 0 || minutes > 0 || seconds > 0)
                {
                        stream << std::setfill('0') << std::setw(2) << seconds << "s:";
                }
                stream << std::setfill('0') << std::setw(3) << miliseconds << "ms";

                return stream.str();
        }
}
