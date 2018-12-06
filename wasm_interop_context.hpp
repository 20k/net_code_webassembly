#ifndef WASM_INTEROP_CONTEXT_HPP_INCLUDED
#define WASM_INTEROP_CONTEXT_HPP_INCLUDED

#include <variant>
#include <nlohmann/json.hpp>
#include <map>
#include <iostream>

struct interop_element
{
    using object = std::map<std::string, std::shared_ptr<interop_element>>;
    using func_ptr = std::function<uint32_t(uint32_t)>;

    ///either we contain data, a function pointer, or we're an object which is a map of elements
    std::variant<nlohmann::json, func_ptr, object> data;

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

namespace runtime
{
    struct store;
}

struct wasm_interop_context
{
    std::map<uint32_t, std::shared_ptr<interop_element>> elems;
    std::map<uint32_t, std::vector<std::shared_ptr<interop_element>>> last_built;

    std::shared_ptr<interop_element> get_back(runtime::store* s, uint32_t gapi);

    uint32_t next_id = 0;
};

#endif // WASM_INTEROP_CONTEXT_HPP_INCLUDED
