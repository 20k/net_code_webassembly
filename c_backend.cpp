#include "c_backend.hpp"

#include "wasm_binary_data.hpp"
#include "LEB.hpp"
#include "serialisable.hpp"
#include "invoke.hpp"

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

/*struct value_stack
{
    std::vector<int> values;
    int last = 0;

    std::vector<int> starts{0};

    void push_stack()
    {
        starts.push_back(values.size());
    }

    void pop_all_values()
    {
        values.resize(starts.back());
    }

    void pop_stack()
    {
        starts.pop_back();
    }

    int& get_next()
    {
        values.push_back(last++);
    }

    int pop_back()
    {
        assert(values.size() > 0);
        int last = values.back();
        values.pop_back();

        return last;
    }

    void push_value(int val)
    {
        values.push_back(val);
    }
};*/

struct c_context
{
    int label_depth = 0;
    std::vector<int> label_arities;
};

std::string define_expr(runtime::store& s, const types::vec<types::instr>& exp, c_context& ctx, int& stack_offset);

///so, frame_abort = true is just a return <last_stack_item> if our arity is > 0
///only thing return arity is used for
///that and functions automatically always return their last stack value
///so basically just do if(return_arity) { return very_first_varible;}

///pops all values off stack when we exit
///have to pass values manually to higher level variables
///each label capture arity has an associated variable
std::string define_label(runtime::store& s, const types::vec<types::instr>& exp, const label& l, c_context& ctx, int stack_offset)
{
    ctx.label_depth++;

    ctx.label_arities.push_back(l.btype.arity());

    std::string fbody;

    ///one stack value argument
    if(l.btype.arity() == 1)
    {
        fbody += l.btype.friendly() + " r_" + std::to_string(ctx.label_depth) + " = 0;";
    }

    if(l.continuation == 2)
        fbody += "//label, 2\nwhile(1){\n";
    else
        fbody += "//label, !2\ndo {\n";

    fbody += "//skip point\n    do {\n"

    if(l.btype.arity() == 1)
    {
        fbody += l.btype.friendly() + " " + get_variable_name(stack_offset++) + " = r_" + std::to_string(ctx.label_depth) + ";";
    }

    define_expr(s, exp, ctx, stack_offset);

    ///so in the event that there's an abort and we're not it, we delete our stack and then back up a level
    ///in the event that there's an abort and we are it, our return value is the next item on the stack

    fbody += "    } ///end skip point\n";

    fbody += "if(abort_stack > 0) {    abort_stack--;\n";

    ///code doesn't need to do anything?
    if(l.continuation == 2)
    {
        ///if we aborted to this loop
        ///loops take no argument
        ///so just keep looping
        assert(l.btype.arity() == 0);
        fbody += "    if(abort_stack == 0) {continue;}";

        ///ok but what if its not us?
        ///Just pop all values on stack? AKA just break outside of this?
        fbody += "    if(abort_stack > 0) {break;}"
    }
    else
    {
        if(l.btype.arity() == 0)
        {
            if()
        }
    }

    fbody += "}\n";


    ctx.label_depth--;
    ctx.label_arities.pop_back();

    if(l.continuation == 2)
        return fbody + "}\n";
    else
        return fbody + "}\nwhile(0);\n";
}

void add_abort(std::string& in)
{
    in += "if(abort_stack > 0)\n    break;\n";
}

///don't need to support eval with frame, do need to support eval with label
std::string define_expr(runtime::store& s, const types::vec<types::instr>& exp, c_context& ctx, int& stack_offset)
{
    size_t len = exp.size();

    std::string ret;

    for(size_t ilen=0; ilen < len; ilen++)
    {
        const types::instr& is = exp[ilen];

        size_t which = is.which;

        ret += "//" + instr_to_str(which) + "\n";

        switch(which)
        {
            case 0x00:
                ret += "assert(false);";
                break;
            case 0x01:
                break;
            /*case 0x02:
                ret += "assert(false);"
                break;
            case 0x03:
                ret += "assert"*/

            case 0x02:
            {
                const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                label l;
                l.btype = sbd.btype;
                l.continuation = 1;

                ret += define_label(s, sbd.first, l, ctx, stack_offset);

                add_abort(ret);
            }

            default:
                ret += "assert(false)";
                break;

        }

        ret += "\n";
    }

    return ret;
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

    function_body += "int return_arity = " + std::to_string(return_arity) + ";\n";
    //function_body += "int current_arity = 0;\n";
    //function_body += "int backup_arity = 0;\n";
    function_body += "int abort_stack = 0;\n";

    function_body += "do {\n";

    for(const types::local& loc : local_types)
    {
        for(uint32_t i=0; i < (uint32_t)loc.n; i++)
        {
            std::string type = loc.type.friendly();

            std::string full_decl = type + " " + get_local_name(current_locals_end++) + " = 0;";

            function_body += full_decl + "\n";
        }
    }

    c_context ctx;

    int soffset = 0;
    function_body += define_expr(s, wasm_func.funct.fnc.e.i, ctx, soffset);

    return function_body + "\nwhile(0);\n}";
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
