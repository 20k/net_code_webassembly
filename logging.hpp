#ifndef LOGGING_HPP_INCLUDED
#define LOGGING_HPP_INCLUDED

#include <iostream>

namespace lg
{
    extern bool enable_logging;

    void log_unpack()
    {

    }

    template<typename U, typename... T>
    void log_unpack(const U& first, const T&... data)
    {
        std::cout << first;

        log_unpack(data...);
    }

    template<typename... T>
    void log(const T&... data)
    {
        if(!enable_logging)
            return;

        log_unpack(data...);
    }
}

#endif // LOGGING_HPP_INCLUDED
