#ifndef LOGGING_HPP_INCLUDED
#define LOGGING_HPP_INCLUDED

#include <iostream>

namespace lg
{
    extern bool enable_logging;

    inline
    void log_unpack()
    {

    }

    template<typename U, typename... T>
    inline
    void log_unpack(const U& first, const T&... data)
    {
        std::cout << first;

        log_unpack(data...);
    }

    template<typename... T>
    inline
    void log(const T&... data)
    {
        if(!enable_logging)
            return;

        log_unpack(data...);

        std::cout << std::endl;
    }

    template<typename... T>
    inline
    void logn(const T&... data)
    {
        if(!enable_logging)
            return;

        log_unpack(data...);
    }

    template<typename T>
    void log_hex_noline(const T& data)
    {
        if(!enable_logging)
            return;

        std::cout << std::hex << data << std::dec;
    }
}

//#define DEBUGGING

#endif // LOGGING_HPP_INCLUDED
