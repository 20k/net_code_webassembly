#ifndef C_BACKEND_HPP_INCLUDED
#define C_BACKEND_HPP_INCLUDED

#include <string>
#include "wasm_binary_data.hpp"

struct data;

std::string get_as_c_program(data d, const std::map<std::string, std::map<std::string, runtime::externval>>& evals);

#endif // C_BACKEND_HPP_INCLUDED
