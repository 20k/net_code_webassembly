#ifndef WASM_BINARY_DATA_HPP_INCLUDED
#define WASM_BINARY_DATA_HPP_INCLUDED

#include "types.hpp"
#include "runtime_types.hpp"

struct wasm_binary_data
{
    void init(data d, const types::vec<runtime::externval>& eval);

    runtime::moduleinst* m_minst = nullptr;
    runtime::store s;
};

#endif // WASM_BINARY_DATA_HPP_INCLUDED
