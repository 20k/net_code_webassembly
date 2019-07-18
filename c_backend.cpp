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

std::string get_global_name(int offset)
{
    return "g_" + std::to_string(offset);
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

std::string function_name(runtime::moduleinst& minst, runtime::funcinst& finst, runtime::funcaddr address)
{
    std::string func_name = "f_" + std::to_string((uint32_t)address);

    if(auto it = minst.funcnames.find(address); it != minst.funcnames.end())
    {
        func_name = it->second;
    }

    return func_name;
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
    std::string func_name = function_name(minst, finst, address);

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

struct value_stack
{
    int safe_offset = 0;
    std::vector<int> stk;

    int get_next()
    {
        int next = safe_offset++;

        stk.push_back(next);

        return next;
    }

    int pop_back()
    {
        if(stk.size() == 0)
        {
            printf("Warning, invalid pop_back\n");
            return -1;
        }

        //throw std::runtime_error("Bad stack");

        int val = stk.back();

        stk.pop_back();

        return val;
    }

    int peek_back()
    {
        if(stk.size() == 0)
        {
            printf("Warning, invalid peek_back\n");
            return -1;
        }

        //throw std::runtime_error("Bad peek");

        return stk.back();
    }

    int get_temporary()
    {
        int val = get_next();
        pop_back();
        return val;
    }
};

struct c_context
{
    int label_depth = 0;
    std::vector<int> label_arities;
};

std::string define_expr(runtime::store& s, const types::vec<types::instr>& exp, c_context& ctx, value_stack& stack_offset, int return_arity, runtime::moduleinst& minst);

///so, frame_abort = true is just a return <last_stack_item> if our arity is > 0
///only thing return arity is used for
///that and functions automatically always return their last stack value
///so basically just do if(return_arity) { return very_first_varible;}

///pops all values off stack when we exit
///have to pass values manually to higher level variables
///each label capture arity has an associated variable
std::string define_label(runtime::store& s, const types::vec<types::instr>& exp, const label& l, c_context& ctx, value_stack stack_offset, int return_arity, runtime::moduleinst& minst)
{
    ctx.label_depth++;

    int destination_stack_val = -1;

    if(l.btype.arity() > 0)
    {
        destination_stack_val = stack_offset.peek_back();
    }

    ctx.label_arities.push_back(l.btype.arity());

    std::string fbody;

    ///one stack value argument
    if(l.btype.arity() == 1)
    {
        fbody += l.btype.friendly() + " r_" + std::to_string(ctx.label_depth) + " = 0;\n";
    }

    if(l.continuation == 2)
        fbody += "//label, 2\nwhile(1){\n";
    else
        fbody += "//label d " + std::to_string(ctx.label_depth) + " , !2\ndo {\n";

    fbody += "    do { //skip point\n";

    /*if(l.btype.arity() == 1)
    {
        fbody += l.btype.friendly() + " " + get_variable_name(stack_offset++) + " = r_" + std::to_string(ctx.label_depth) + ";";
    }*/

    fbody += define_expr(s, exp, ctx, stack_offset, return_arity, minst);

    ///so in the event that there's an abort and we're not it, we delete our stack and then back up a level
    ///in the event that there's an abort and we are it, our return value is the next item on the stack

    fbody += "    } //end skip point\n";

    fbody += "if(abort_stack > 0) {\n    abort_stack--;\n";

    ///code doesn't need to do anything?
    if(l.continuation == 2)
    {
        ///if we aborted to this loop
        ///loops take no argument
        ///so just keep looping
        assert(l.btype.arity() == 0);
        fbody += "    if(abort_stack == 0) {continue;}\n";

        ///ok but what if its not us?
        ///Just pop all values on stack? AKA just break outside of this?
        fbody += "    if(abort_stack > 0) {break;}\n";
    }
    else
    {
        if(l.btype.arity() == 0)
        {
            fbody += "    if(abort_stack > 0) {break;}\n";
            fbody += "    if(abort_stack == 0) {break;}\n"; ///no capture arity for me?
        }
        else
        {
            fbody += "    if(abort_stack > 0) {break;}\n";

            ///uuh ok what to do with return value
            ///so basically r_labelidx is going to be set to the value we want right?
            ///so we need to take the stack value that we're meant to be popping into
            ///which is... stack_offset
            fbody += "    if(abort_stack == 0) { " + get_variable_name(destination_stack_val) + " = r_" + std::to_string(ctx.label_depth) + "; break;}\n";
        }
    }

    fbody += "}";

    if(l.btype.arity() > 0)
    {
        fbody += " else { \n";

        fbody += get_variable_name(destination_stack_val) + " = " + get_variable_name(stack_offset.pop_back()) + ";\n}";
    }

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

template<typename T, typename U>
std::string c_mem_load(runtime::store& s, const types::memarg& arg, value_stack& stack_offset, runtime::moduleinst& minst)
{
    std::string ret;

    if(s.mems.size() < 1)
        throw std::runtime_error("No such mem idx 0");

    runtime::memaddr addr = minst.memaddrs[0];

    uint32_t raw_addr = (uint32_t)addr;

    if(raw_addr >= (uint32_t)s.mems.size())
        throw std::runtime_error("raw_addr >= s.mems.size()");

    int top_val = stack_offset.pop_back();

    runtime::value utemp;
    utemp.set(U());

    runtime::value ttemp;
    ttemp.set(T());

    int u_1 = stack_offset.get_temporary();
    int t_1 = stack_offset.get_next();

    ret += "static_assert(typeid(" + get_variable_name(top_val) + ") == typeid(i32));\n";

    std::string sum = std::to_string((uint32_t)arg.offset) + " + " + get_variable_name(top_val);

    ret += "if(" + sum + " + sizeof(" + utemp.friendly() + ") >= mem_0.size()) {assert(false);}\n";
    ret += "if(" + sum + " < 0) {assert(false);}\n";
    ret += utemp.friendly() + " " + get_variable_name(u_1) + " = 0;\n";
    ret += "memcpy(&" + get_variable_name(u_1) + ", &mem_0[" + sum + "], sizeof(" + utemp.friendly() + "));\n";

    ret += ttemp.friendly() + " " + get_variable_name(t_1) + " = (" + ttemp.friendly() + ")" + get_variable_name(u_1) + ";\n";

    return ret;
}

template<typename T, int bytes>
std::string c_mem_store(runtime::store& s, const types::memarg& arg, value_stack& stack_offset, runtime::moduleinst& minst)
{
    static_assert(bytes <= sizeof(T));

    std::string ret;

    if(s.mems.size() < 1)
        throw std::runtime_error("No such mem idx 0");

    runtime::memaddr addr = minst.memaddrs[0];

    uint32_t raw_addr = (uint32_t)addr;

    if(raw_addr >= (uint32_t)s.mems.size())
        throw std::runtime_error("raw_addr >= s.mems.size()");

    int store_value = stack_offset.pop_back();
    int store_bytes = stack_offset.pop_back();

    runtime::value ttemp;
    ttemp.set(T());

    std::string sum = std::to_string((uint32_t)arg.offset) + " + " + std::to_string(store_bytes);

    ret += "static_assert(typeid(" + get_variable_name(store_bytes) + ") == typeid(i32));\n";
    ret += "if(" + sum + " + " + std::to_string(bytes) + " >= mem_0.size()) {assert(false);}\n";
    ret += "if(" + sum + " < 0) {assert(false);}\n";

    ret += "static_assert(" + std::to_string(bytes) + " <= sizeof(" + get_variable_name(store_value) + "));\n";

    ret += "memcpy(&mem_0[" + sum + "], (char*)&" + get_variable_name(store_value) + ", " + std::to_string(bytes) + ");\n";

    return ret;
}

#define C_MEM_LOAD(x, y) ret += c_mem_load<x, y>(s, std::get<types::memarg>(is.dat), stack_offset, minst); break;
#define C_MEM_STORE(x, y) ret += c_mem_store<x, y>(s, std::get<types::memarg>(is.dat), stack_offset, minst); break;

std::string sfjump(c_context& ctx, value_stack& stack_offset, types::labelidx lidx)
{
    std::string ret;

    int next_label_depth = ctx.label_depth - (int)(uint32_t)lidx;

    int nidx = next_label_depth - 1;

    assert(nidx >= 0 && nidx < (int)ctx.label_arities.size());

    int arity = ctx.label_arities[nidx];

    if(arity > 0)
        ret += "r_" + std::to_string(next_label_depth) + " = " + get_variable_name(stack_offset.pop_back()) + ";\n";

    ret += "abort_stack = " + std::to_string((uint32_t)lidx + 1) + "\n";

    return ret;
}

std::string and_push(const std::string& in, value_stack& stack_offset, types::valtype type)
{
    return type.friendly() + " " + get_variable_name(stack_offset.get_next()) + " = " + in;
}

std::string auto_push(const std::string& in, value_stack& stack_offset)
{
    return "auto " + get_variable_name(stack_offset.get_next()) + " = " + in;
}

std::string invoke_function(runtime::store& s, c_context& ctx, value_stack& stack_offset, runtime::moduleinst& minst, runtime::funcaddr address, bool push)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    int num_args = ftype.params.size();
    int num_rets = ftype.results.size();

    std::string to_call = function_name(minst, finst, address);

    to_call += "(";

    std::vector<int> rargs;

    for(int i=0; i < num_args; i++)
    {
        rargs.push_back(stack_offset.pop_back());
    }

    auto args = rargs;
    std::reverse(args.begin(), args.end());

    for(int i=0; i < num_args; i++)
    {
        to_call += get_variable_name(args[i]);

        if(i != num_args-1)
            to_call += ", ";
    }

    to_call += ");";

    std::string ret = to_call;

    if(num_rets > 0 && push)
    {
        ret = and_push(to_call, stack_offset, ftype.results[0]);
    }

    return ret + "\n";
}

///don't need to support eval with frame, do need to support eval with label
std::string define_expr(runtime::store& s, const types::vec<types::instr>& exp, c_context& ctx, value_stack& stack_offset, int return_arity, runtime::moduleinst& minst)
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

            ///block
            case 0x02:
            {
                const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                label l;
                l.btype = sbd.btype;
                l.continuation = 1;

                if(l.btype.arity() > 0)
                    ret += sbd.btype.friendly() + " " + get_variable_name(stack_offset.get_next()) + " = 0\n";

                ret += define_label(s, sbd.first, l, ctx, stack_offset, return_arity, minst);

                add_abort(ret);
                break;
            }

            ///loop
            case 0x03:
            {
                const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                label l;
                l.btype = sbd.btype;
                l.continuation = 2;

                assert(l.btype.arity() == 0);

                ret += define_label(s, sbd.first, l, ctx, stack_offset, return_arity, minst);

                add_abort(ret);

                break;
            }

            ///if
            case 0x04:
            {
                const types::double_branch_data& dbd = std::get<types::double_branch_data>(is.dat);

                label l;
                l.btype = dbd.btype;
                l.continuation = 3;

                int branch_variable = stack_offset.pop_back();

                if(l.btype.arity() > 0)
                    ret += dbd.btype.friendly() + " " + get_variable_name(stack_offset.get_next()) + " = 0\n";

                ret += "if(" + get_variable_name(branch_variable) + " != 0)\n{";

                ret += define_label(s, dbd.first, l, ctx, stack_offset, return_arity, minst);

                ret += "} else { ";

                ret += define_label(s, dbd.second, l, ctx, stack_offset, return_arity, minst);

                ret += "} //endif";

                add_abort(ret);

                break;
            }

            ///br
            case 0x0C:
            {
                types::labelidx lidx = std::get<types::labelidx>(is.dat);

                ret += sfjump(ctx, stack_offset, lidx);

                add_abort(ret);

                break;
            }

            ///br_if
            case 0x0D:
            {
                int decision_variable = stack_offset.pop_back();

                ret = "if(" + get_variable_name(decision_variable) + " != 0) {\n";

                types::labelidx lidx = std::get<types::labelidx>(is.dat);

                ret += sfjump(ctx, stack_offset, lidx);

                add_abort(ret);

                ret += "}";

                break;
            }

            ///br_t
            case 0x0E:
            {
                const types::br_table_data& br_td = std::get<types::br_table_data>(is.dat);

                const types::vec<types::labelidx>& labels = br_td.labels;

                int decision_variable = stack_offset.pop_back();

                for(int i=0; i < (int)labels.size(); i++)
                {
                    if(i == 0)
                    {
                        ret += "if(";
                    }
                    else
                    {
                        ret += "else if(";
                    }

                    ret += get_variable_name(decision_variable) +  " == " + std::to_string(i) + ") {\n";

                    types::labelidx lidx = labels[i];

                    ret += sfjump(ctx, stack_offset, lidx);

                    ret += "}";
                }

                if(labels.size() > 0)
                    ret += "else {\n";
                else
                    ret += "{\n";

                types::labelidx lidx = br_td.fin;

                ret += sfjump(ctx, stack_offset, lidx);

                ret += "}";

                stack_offset.pop_back();

                printf("Warning: Generating br_td which isn't well tested\n");

                break;
            }

            ///return
            case 0x0F:
            {
                if(return_arity > 0)
                {
                    ret += "return " + get_variable_name(stack_offset.pop_back()) + ";";
                }
                else
                {
                    ret += "return;";
                }

                break;
            }

            ///call
            case 0x10:
            {
                types::funcidx fidx = std::get<types::funcidx>(is.dat);

                uint32_t idx = (uint32_t)fidx;

                if(idx >= (uint32_t)minst.funcaddrs.size())
                    throw std::runtime_error("Bad fidx in 0x10 [call]");

                ret += invoke_function(s, ctx, stack_offset, minst, minst.funcaddrs[idx], true);

                add_abort(ret);

                break;
            }

            ///calli
            case 0x11:
            {
                types::typeidx found_tidx = std::get<types::typeidx>(is.dat);

                if(minst.tableaddrs.size() < 1)
                    throw std::runtime_error("Failed table addr < 1 in indirect call");

                runtime::tableaddr taddr = minst.tableaddrs[0];

                uint32_t tidx = (uint32_t)taddr;

                if(tidx >= (uint32_t)s.tables.size())
                    throw std::runtime_error("Bad tidx in indirect call");

                runtime::tableinst& tinst = s.tables[tidx];

                uint32_t type_idx = (uint32_t)found_tidx;

                if(type_idx >= (uint32_t)minst.typel.size())
                    throw std::runtime_error("Bad fidx");

                types::functype ft_expect = minst.typel[type_idx];

                int check_variable = stack_offset.pop_back();

                int declared_return = -1;

                if(ft_expect.results.size() > 0)
                {
                    declared_return = stack_offset.get_next();;
                    ret += ft_expect.results[0].friendly() + " " + get_variable_name(declared_return) + " = 0;\n";
                }

                for(int i=0; i < (int)tinst.elem.size(); i++)
                {
                    if(i == 0)
                        ret += "if(";
                    else
                        ret += "else if(";

                    ret += get_variable_name(check_variable) + " == " + std::to_string(i) + ") {\n";

                    runtime::funcaddr runtime_addr = tinst.elem[i].addr.value();

                    uint32_t runtime_idx = (uint32_t)runtime_addr;

                    if(runtime_idx >= (uint32_t)s.funcs.size())
                        throw std::runtime_error("Runtime idx oob (validation)");

                    runtime::funcinst& finst = s.funcs[runtime_idx];

                    types::functype ft_actual = finst.type;

                    if(types::funcs_equal(ft_actual, ft_expect))
                    {
                        if(ft_expect.results.size() > 0)
                            ret += get_variable_name(declared_return) + " = " + invoke_function(s, ctx, stack_offset, minst, runtime_addr, false) + ";\n";
                        else
                            ret += invoke_function(s, ctx, stack_offset, minst, runtime_addr, false) + ";\n";
                    }
                    else
                    {
                        ret += "assert(false);\n";
                    }

                    ret += "}\n";
                }

                if(tinst.elem.size() > 0)
                    ret += "else {assert(false);}\n";
                else
                    ret += "{assert(false);}\n";

                break;
            }

            case 0x1A:
            {
                stack_offset.pop_back();
                break;
            }

            case 0x1B:
            {
                int decider = stack_offset.pop_back();
                int second = stack_offset.pop_back();
                int first = stack_offset.pop_back();

                ///next = decider ? first : second;
                ret += auto_push(get_variable_name(decider) + " ? " + get_variable_name(first) + " : " + get_variable_name(second) + ";\n", stack_offset);
                break;
            }

            ///get_local
            case 0x20:
            {
                types::localidx idx = std::get<types::localidx>(is.dat);

                ret += auto_push(get_local_name((uint32_t)idx) + ";\n", stack_offset);

                break;
            }

            ///set_local
            case 0x21:
            {
                types::localidx idx = std::get<types::localidx>(is.dat);

                int top_val = stack_offset.pop_back();

                ///no need to check because top_val >= activate.f.locals.size() is a compile error
                ret += get_local_name((uint32_t)idx) + " = " + get_variable_name(top_val) + ";\n";

                break;
            }

            ///tee_local
            case 0x22:
            {
                types::localidx idx = std::get<types::localidx>(is.dat);

                ///the pop/push here is a bit unnecessary
                int top_val = stack_offset.pop_back();

                ret += get_local_name((uint32_t)idx) + " = " + get_variable_name(top_val) + ";\n";
                ret += auto_push(get_variable_name(top_val) + ";\n", stack_offset);

                break;
            }

            ///get_global
            case 0x23:
            {
                types::globalidx gidx = std::get<types::globalidx>(is.dat);

                uint32_t idx = (uint32_t)gidx;

                if(idx >= (uint32_t)minst.globaladdrs.size())
                    throw std::runtime_error("bad idx in get_global");

                runtime::globaladdr addr = minst.globaladdrs[idx];

                if((uint32_t)addr >= (uint32_t)s.globals.size())
                    throw std::runtime_error("bad addr in get_global");

                runtime::globalinst& glob = s.globals[(uint32_t)addr];

                runtime::value val = glob.val;

                int next_var = stack_offset.get_next();

                ret += val.friendly() + " " + get_variable_name(next_var) + " = " + get_global_name((uint32_t)addr) + ";\n";

                break;
            }

            ///get_global
            case 0x24:
            {
                types::globalidx gidx = std::get<types::globalidx>(is.dat);

                uint32_t idx = (uint32_t)gidx;

                if(idx >= (uint32_t)minst.globaladdrs.size())
                    throw std::runtime_error("bad idx in set_global");

                runtime::globaladdr addr = minst.globaladdrs[idx];

                if((uint32_t)addr >= (uint32_t)s.globals.size())
                    throw std::runtime_error("bad addr in set_global");

                runtime::globalinst& glob = s.globals[(uint32_t)addr];

                runtime::value val = glob.val;

                int set_var = stack_offset.pop_back();

                get_global_name((uint32_t)addr) + " = " + get_variable_name(set_var) + ";\n";

                break;
            }

            case 0x28:
                C_MEM_LOAD(uint32_t, uint32_t);
            case 0x29:
                C_MEM_LOAD(uint64_t, uint64_t);
            case 0x2A:
                C_MEM_LOAD(float, float);
            case 0x2B:
                C_MEM_LOAD(double, double);
            case 0x2C:
                C_MEM_LOAD(int32_t, int8_t);
            case 0x2D:
                C_MEM_LOAD(uint32_t, uint8_t);
            case 0x2E:
                C_MEM_LOAD(int32_t, int16_t);
            case 0x2F:
                C_MEM_LOAD(uint32_t, uint16_t);
            case 0x30:
                C_MEM_LOAD(int64_t, int8_t);
            case 0x31:
                C_MEM_LOAD(uint64_t, uint8_t);
            case 0x32:
                C_MEM_LOAD(int64_t, int16_t);
            case 0x33:
                C_MEM_LOAD(uint64_t, uint16_t);
            case 0x34:
                C_MEM_LOAD(int64_t, int32_t);
            case 0x35:
                C_MEM_LOAD(uint64_t, uint32_t);


            case 0x36:
                C_MEM_STORE(uint32_t, 4);
            case 0x37:
                C_MEM_STORE(uint64_t, 8);
            case 0x38:
                C_MEM_STORE(float, 4);
            case 0x39:
                C_MEM_STORE(double, 8);
            case 0x3A:
                C_MEM_STORE(uint32_t, 1);
            case 0x3B:
                C_MEM_STORE(uint32_t, 2);
            case 0x3C:
                C_MEM_STORE(uint64_t, 1);
            case 0x3D:
                C_MEM_STORE(uint64_t, 2);
            case 0x3E:
                C_MEM_STORE(uint64_t, 4);


            default:
                ret += "assert(false); //fellthrough";
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

    //function_body += "int return_arity = " + std::to_string(return_arity) + ";\n";
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

    value_stack soffset;
    function_body += define_expr(s, wasm_func.funct.fnc.e.i, ctx, soffset, return_arity, minst);

    return function_body + "\nwhile(0);\n}";
}

std::string compile_top_level(runtime::store& s, runtime::funcaddr address, runtime::moduleinst& minst, const types::vec<std::string>& vals)
{
    ///need to inject memory

    std::string res =
    R"(
#include <vector>
#include <cstdint>
#include <typeinfo>

using i32 = uint32_t;
using i64 = uint64_t;
using f32 = float;
using f64 = double;
using empty = void;

)";

    for(int i=0; i < s.mems.size(); i++)
    {
        runtime::meminst& inst = s.mems[i];

        uint64_t mem_size = inst.dat.size();

        res += "std::vector<char> mem_" + std::to_string(i) + "(" + std::to_string(mem_size) + ");\n";
    }

    for(runtime::globaladdr addr : minst.globaladdrs)
    {
        uint32_t idx = (uint32_t)addr;
        runtime::globalinst& glob = s.globals[(uint32_t)addr];

        res += glob.val.friendly() + " " + get_global_name(idx) + " = " + glob.val.friendly_val() + ";\n";
    }

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
