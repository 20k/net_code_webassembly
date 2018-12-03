#ifndef INJECTION_HELPERS_HPP_INCLUDED
#define INJECTION_HELPERS_HPP_INCLUDED

#include <map>
#include <string>
#include "runtime_types.hpp"

std::map<std::string, std::map<std::string, runtime::externval>> get_env_helpers(runtime::store& s);

#endif // INJECTION_HELPERS_HPP_INCLUDED
