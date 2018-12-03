#ifndef WASM_INTEROP_CONTEXT_HPP_INCLUDED
#define WASM_INTEROP_CONTEXT_HPP_INCLUDED

#include <variant>
#include <nlohmann/json.hpp>
#include <map>

struct interop_element
{
    std::map<std::string, std::variant<nlohmann::json, void(*)(), std::shared_ptr<interop_element>>> data;
};

struct wasm_interop_context
{
    std::map<uint32_t, interop_element> elems;
};

#endif // WASM_INTEROP_CONTEXT_HPP_INCLUDED
