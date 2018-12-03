#ifndef WASM_INTEROP_CONTEXT_HPP_INCLUDED
#define WASM_INTEROP_CONTEXT_HPP_INCLUDED

#include <variant>
#include <nlohmann/json.hpp>
#include <map>
#include <iostream>

struct interop_element
{
    using object = std::map<std::string, std::shared_ptr<interop_element>>;

    ///either we contain data, a function pointer, or we're an object which is a map of elements
    std::variant<nlohmann::json, void(*)(), object> data;

    void update_object_element(const std::string& key, std::shared_ptr<interop_element>& val)
    {
        if(!std::holds_alternative<interop_element::object>(data))
        {
            data = interop_element::object{{key, val}};
        }
        else
        {
            std::get<interop_element::object>(data)[key] = val;
        }
    }

    template<typename T>
    void update_object_element(const std::string& key, const T& val)
    {
        std::shared_ptr<interop_element> interop;
        interop = std::make_shared<interop_element>();

        update_object_element(key, interop);

        interop->data = val;
    }

    //std::map<std::string, std::variant<nlohmann::json, void(*)(), std::shared_ptr<interop_element>>> data;
};

struct wasm_interop_context
{
    std::map<uint32_t, std::shared_ptr<interop_element>> elems;
    std::map<uint32_t, std::vector<std::shared_ptr<interop_element>>> last_built;
};

#endif // WASM_INTEROP_CONTEXT_HPP_INCLUDED
