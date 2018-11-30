#ifndef COMPILE_HPP_INCLUDED
#define COMPILE_HPP_INCLUDED

#include <string>

struct compile_result
{
    std::string data;
    std::string err;
};

compile_result compile(const std::string& file);

#endif // COMPILE_HPP_INCLUDED
