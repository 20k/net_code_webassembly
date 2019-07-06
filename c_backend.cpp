#include "c_backend.hpp"

#include "wasm_binary_data.hpp"
#include "LEB.hpp"
#include "serialisable.hpp"

std::string compile_function(runtime::funcaddr faddr, runtime::moduleinst& minst, const types::vec<runtime::value>& vals)
{


    return "";
}

std::string get_as_c_program(data d, const std::map<std::string, std::map<std::string, runtime::externval>>& evals)
{
    wasm_binary_data bin;
    bin.init(d, evals);

    for(runtime::exportinst& einst : bin.m_minst->exports)
    {
        if(einst.name == "main")
        {
            return compile_function(std::get<runtime::funcaddr>(einst.value.val), *bin.m_minst, {});
        }
    }

    throw std::runtime_error("Did not find main");
}
