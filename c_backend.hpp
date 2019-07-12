#ifndef C_BACKEND_HPP_INCLUDED
#define C_BACKEND_HPP_INCLUDED

#include <string>
#include "wasm_binary_data.hpp"

struct data;

std::string get_as_c_program(wasm_binary_data& bin);

#endif // C_BACKEND_HPP_INCLUDED
