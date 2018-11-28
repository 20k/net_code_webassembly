#ifndef WASM_BINARY_DATA_HPP_INCLUDED
#define WASM_BINARY_DATA_HPP_INCLUDED

#include "types.hpp"
#include "runtime_types.hpp"
#include <map>

struct wasm_binary_data
{
    //void init(data d, const types::vec<runtime::externval>& eval);

    void init(data d, const std::map<std::string, std::map<std::string, runtime::externval>>& evals);

    runtime::moduleinst* m_minst = nullptr;
    runtime::store s;
};

#endif // WASM_BINARY_DATA_HPP_INCLUDED
