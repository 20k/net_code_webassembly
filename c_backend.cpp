#include "c_backend.hpp"

#include "wasm_binary_data.hpp"
#include "LEB.hpp"
#include "serialisable.hpp"

std::string get_variable_name(int offset)
{
    return "a_" + std::to_string(offset);
}

std::string get_local_name(int offset)
{
    return "l_" + std::to_string(offset);
}

std::string join_commawise(const std::vector<std::string>& in)
{
    std::string ret;

    for(int i=0; i < (int)in.size()-1; i++)
    {
        ret += in[i] + ", ";
    }

    if(in.size() > 0)
    {
        ret += in.back();
    }

    return ret;
}

//, const types::vec<std::string>& vals

///need to import function names
std::string declare_function(runtime::store& s, runtime::funcaddr address, runtime::moduleinst& minst)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    /*if(!std::holds_alternative<runtime::webasm_func>(finst.funct))
        throw std::runtime_error("unimplemented");*/

    bool external = !std::holds_alternative<runtime::webasm_func>(finst.funct);

    /*if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch received " + std::to_string(vals.size()) + " but expected " + std::to_string(ftype.params.size()));*/

    std::string func_return = ftype.results.size() > 0 ? ftype.results[0].friendly() : "void";
    std::string func_name = "f_" + std::to_string(adr);

    if(auto it = minst.funcnames.find(address); it != minst.funcnames.end())
    {
        func_name = it->second;
    }

    std::vector<std::string> args;

    for(int aidx = 0; aidx < (int)ftype.params.size(); aidx++)
    {
        types::valtype type = ftype.params[aidx];

        args.push_back(type.friendly() + " " + get_local_name(aidx));
    }

    std::string external_str = external ? "extern " : "";

    return external_str + func_return + " " + func_name + "(" + join_commawise(args) + ")";


    ///so
    ///need to go through function code
    ///get code vector
    ///go through code vector generating c code that's compliant with webasm for each thing
    ///careful of ifs because this is not a linear code segment
}

///don't need to support eval with frame, do need to support eval with label
std::string define_expr(runtime::store& s, const types::vec<types::instr>& exp, int return_arity)
{
    size_t len = exp.size();


}

std::string define_function(runtime::store& s, runtime::funcaddr address, runtime::moduleinst& minst)
{
    std::string sig = declare_function(s, address, minst);

    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    bool external = !std::holds_alternative<runtime::webasm_func>(finst.funct);

    if(external)
        return "";

    runtime::webasm_func& wasm_func = std::get<runtime::webasm_func>(finst.funct);
    const types::vec<types::local>& local_types = wasm_func.funct.fnc.locals;

    int current_stack_end = 0;
    int current_locals_end = ftype.params.size();

    std::string function_body = sig + " {\n";

    int return_arity = ftype.results.size() > 0;

    for(const types::local& loc : local_types)
    {
        for(uint32_t i=0; i < (uint32_t)loc.n; i++)
        {
            std::string type = loc.type.friendly();

            std::string full_decl = type + " " + get_local_name(current_locals_end++) + " = 0;";

            function_body += full_decl + "\n";
        }
    }

    return function_body + "\n}";
}

std::string compile_top_level(runtime::store& s, runtime::funcaddr address, runtime::moduleinst& minst, const types::vec<std::string>& vals)
{
    ///need to inject memory

    std::string res =
    R"(
#include <vector>
#include <cstdint>

using i32 = uint32_t;
using i64 = uint64_t;
using f32 = float;
using f64 = double;
using empty = void;

)";

    //for(runtime::funcinst& finst : s.funcs)

    runtime::funcaddr base;
    base.val = 0;

    for(; (uint32_t)base < s.funcs.size(); base.val++)
    {
        res += declare_function(s, base, minst) + ";\n\n";
    }

    base.val = 0;

    for(; (uint32_t)base < s.funcs.size(); base.val++)
    {
        res += define_function(s, base, minst) + "\n\n";
    }

    return res;
}


std::string get_as_c_program(wasm_binary_data& bin)
{
    for(runtime::exportinst& einst : bin.m_minst->exports)
    {
        if(einst.name == "main")
        {
            return compile_top_level(bin.s, std::get<runtime::funcaddr>(einst.value.val), *bin.m_minst, {});
        }
    }

    throw std::runtime_error("Did not find main");
}
