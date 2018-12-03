#include "wasm_interop_context.hpp"
#include "runtime_types.hpp"

std::shared_ptr<interop_element> wasm_interop_context::get_back(runtime::store* s, uint32_t gapi)
{
    auto it = s->interop_context.last_built.find(gapi);

    if(it == s->interop_context.last_built.end())
        throw std::runtime_error("Bad object in basic string");

    if(it->second.size() < 1)
        throw std::runtime_error("No object to put on");

    std::shared_ptr<interop_element> ielem = it->second.back();

    return ielem;
}
