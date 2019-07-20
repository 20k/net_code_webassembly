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

///TODO: Global variables might have side effects? unsure
std::string get_global_name(int offset)
{
    return "g_" + std::to_string(offset);
}

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

#include "c_basic_ops.hpp"

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

    for(runtime::exportinst& einst : minst.exports)
    {
        if(!std::holds_alternative<runtime::funcaddr>(einst.value.val))
            continue;

        runtime::funcaddr faddr = std::get<runtime::funcaddr>(einst.value.val);

        if((uint32_t)faddr == (uint32_t)address)
            return einst.name.friendly();
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

    if(func_name == "main")
        func_return = "int";

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

    fbody += "    } while(0); //end skip point\n";

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
        //fbody += "    if(abort_stack > 0) {break;}\n";
    }
    else
    {
        if(l.btype.arity() == 0)
        {
            //fbody += "    if(abort_stack > 0) {break;}\n";
            //fbody += "    if(abort_stack == 0) {break;}\n"; ///no capture arity for me?
        }
        else
        {
            //fbody += "    if(abort_stack > 0) {break;}\n";

            ///uuh ok what to do with return value
            ///so basically r_labelidx is going to be set to the value we want right?
            ///so we need to take the stack value that we're meant to be popping into
            ///which is... stack_offset
            fbody += "    if(abort_stack == 0) { " + get_variable_name(destination_stack_val) + " = r_" + std::to_string(ctx.label_depth) + "; break;}\n";
        }
    }

    fbody += "}\n";

    if(l.btype.arity() > 0)
    {
        fbody += " else { \n";

        fbody += get_variable_name(destination_stack_val) + " = " + get_variable_name(stack_offset.pop_back()) + ";\n}";
    }

    ///reached end of block
    fbody += "    break;\n";

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

    ret += "static_assert(std::is_same<decltype(" + get_variable_name(top_val) + "), i32>::value);\n";

    std::string sum = std::to_string((uint32_t)arg.offset) + " + " + get_variable_name(top_val);

    ret += "if(" + sum + " + sizeof(" + utemp.friendly() + ") > mem_0.size()) {assert(false);}\n";
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

    std::string sum = std::to_string((uint32_t)arg.offset) + " + " + get_variable_name(store_bytes);

    //ret += "printf(\"EAB %i %i\\n\", " + sum + ", mem_0.size());\n";
    ret += "static_assert(std::is_same<decltype(" + get_variable_name(store_bytes) + "), i32>::value);\n";
    ret += "if(" + sum + " + " + std::to_string(bytes) + " > mem_0.size()) {assert(false);}\n";
    ret += "if(" + sum + " < 0) {assert(false);}\n";

    ret += "static_assert(" + std::to_string(bytes) + " <= sizeof(" + get_variable_name(store_value) + "));\n";

    ret += "memcpy(&mem_0[" + sum + "], (char*)&" + get_variable_name(store_value) + ", " + std::to_string(bytes) + ");\n";

    return ret;
}

#define C_MEM_LOAD(x, y) ret += c_mem_load<x, y>(s, std::get<types::memarg>(is.dat), stack_offset, minst); break;
#define C_MEM_STORE(x, y) ret += c_mem_store<x, y>(s, std::get<types::memarg>(is.dat), stack_offset, minst); break;

#define C_PUSH_CONSTANT(xtype)\
        { \
            int next = stack_offset.get_next(); \
            types::valtype type; \
            type.set<xtype>(); \
            ret += type.friendly() + " " + get_variable_name(next) + " = (" + type.friendly() + ")" + std::to_string(std::get<xtype>(is.dat).val) + ";\n"; \
            break; \
        }

#define ASSERT_TYPE(x, y) ret += "static_assert(std::is_same<decltype(" + get_variable_name(x) + "), " + types::friendly(y()) + ">::value);\n";

#define ASSERT_SAME_TYPES(x, y) ret += "static_assert(std::is_same<decltype(" + x + "), decltype(" + y + ")>::value);\n";

#define IS_SAME(x, y) std::string("std::is_same<decltype(" + x + "), " + types::friendly(y()) + ">::value")
#define ASSERT_VALID_TYPE(x) ret += "static_assert(" + IS_SAME(x, types::i32) + " || " + IS_SAME(x, types::i64) + " || " + IS_SAME(x, types::f32) + " || " + IS_SAME(x, types::f64) + ");\n";

#define C_POPAT(x, y, z){int val_1 = stack_offset.pop_back(); ASSERT_TYPE(val_1, y); ret += and_push(x(val_1), stack_offset, z()); break;}
#define C_POPBT(x, y, z){int val_2 = stack_offset.pop_back(); int val_1 = stack_offset.pop_back(); ASSERT_TYPE(val_1, y); ASSERT_TYPE(val_2, y); ret += and_push(x(val_1, val_2), stack_offset, z()); break;}

#define C_POPBT_RAW(x, y, z){int val_2 = stack_offset.pop_back(); int val_1 = stack_offset.pop_back(); ASSERT_TYPE(val_1, y); ASSERT_TYPE(val_2, y); ret += x(val_1, val_2, stack_offset, z()); break;}

std::string sfjump(c_context& ctx, value_stack& stack_offset, types::labelidx lidx, bool& would_consume)
{
    std::string ret;

    int next_label_depth = ctx.label_depth - (int)(uint32_t)lidx;

    int nidx = next_label_depth - 1;

    assert(nidx >= 0 && nidx < (int)ctx.label_arities.size());

    int arity = ctx.label_arities[nidx];

    if(arity > 0)
    {
        ret += "r_" + std::to_string(next_label_depth) + " = " + get_variable_name(stack_offset.peek_back()) + ";\n";
        would_consume = true;
    }

    ret += "abort_stack = " + std::to_string((uint32_t)lidx + 1) + ";\n";

    return ret;
}

std::string and_push(const std::string& in, value_stack& stack_offset, types::valtype type)
{
    return type.friendly() + " " + get_variable_name(stack_offset.get_next()) + " = " + in;
}

template<typename T>
std::string and_push(const std::string& in, value_stack& stack_offset, T t)
{
    types::valtype vt;
    vt.set<T>();

    return and_push(in, stack_offset, vt);
}

std::string auto_push(const std::string& in, value_stack& stack_offset)
{
    return "auto " + get_variable_name(stack_offset.get_next()) + " = " + in;
}

std::string invoke_function(runtime::store& s, c_context& ctx, value_stack stack_offset, runtime::moduleinst& minst, runtime::funcaddr address)
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

    to_call += ")";

    return to_call;
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

        #ifdef INSTRUCTION_TRACING
        if(stack_offset.peek_back() != -1)
            ret += "std::cout << " + get_variable_name(stack_offset.peek_back()) + " << std::endl;\n";

        ret += "printf(\"" + instr_to_str(which) + "\\n\");\n";
        #endif // INSTRUCTION_TRACING

        switch(which)
        {
            case 0x00:
                ret += "assert(false);\n__builtin_unreachable();";
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

                ///the reason why this abort is here is because
                ///basically if we still need to abort upwards, we don't want to execute any more
                ///code
                if(ctx.label_depth > 0)
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

                if(ctx.label_depth > 0)
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

                if(ctx.label_depth > 0)
                    add_abort(ret);

                break;
            }

            ///br
            case 0x0C:
            {
                types::labelidx lidx = std::get<types::labelidx>(is.dat);

                bool would_consume = false;
                ret += sfjump(ctx, stack_offset, lidx, would_consume);

                if(would_consume)
                    stack_offset.pop_back();

                add_abort(ret);

                break;
            }

            ///br_if
            case 0x0D:
            {
                int decision_variable = stack_offset.pop_back();

                ret += "if(" + get_variable_name(decision_variable) + " != 0) {\n";

                types::labelidx lidx = std::get<types::labelidx>(is.dat);

                bool would_consume = false;
                ret += sfjump(ctx, stack_offset, lidx, would_consume);

                if(would_consume)
                    stack_offset.pop_back();

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

                bool would_consume = false;

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

                    ret += sfjump(ctx, stack_offset, lidx, would_consume);

                    ret += "}";
                }

                if(labels.size() > 0)
                    ret += "else {\n";
                else
                    ret += "{\n";

                types::labelidx lidx = br_td.fin;

                ret += sfjump(ctx, stack_offset, lidx, would_consume);

                ret += "}";

                if(would_consume)
                    stack_offset.pop_back();

                //stack_offset.pop_back();

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

                runtime::funcaddr address = minst.funcaddrs[idx];

                uint32_t adr = (uint32_t)address;

                if(adr >= (uint32_t)s.funcs.size())
                    throw std::runtime_error("Adr out of bounds");

                runtime::funcinst& finst = s.funcs[adr];

                types::functype ftype = finst.type;

                std::string func_call = invoke_function(s, ctx, stack_offset, minst, address) + ";\n";

                for(int i=0; i < (int)ftype.params.size(); i++)
                {
                    stack_offset.pop_back();
                }

                if(ftype.results.size() > 0)
                {
                    ret += and_push(func_call, stack_offset, ftype.results[0]);
                }
                else
                {
                    ret += func_call;
                }

                break;
            }

            ///call_i
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

                    if(tinst.elem[i].addr.has_value())
                    {
                        runtime::funcaddr runtime_addr = tinst.elem[i].addr.value();

                        uint32_t runtime_idx = (uint32_t)runtime_addr;

                        if(runtime_idx >= (uint32_t)s.funcs.size())
                            throw std::runtime_error("Runtime idx oob (validation)");

                        runtime::funcinst& finst = s.funcs[runtime_idx];

                        types::functype ft_actual = finst.type;

                        if(types::funcs_equal(ft_actual, ft_expect))
                        {
                            if(ft_expect.results.size() > 0)
                                ret += get_variable_name(declared_return) + " = " + invoke_function(s, ctx, stack_offset, minst, runtime_addr) + ";\n";
                            else
                                ret += invoke_function(s, ctx, stack_offset, minst, runtime_addr) + ";\n";
                        }
                        else
                        {
                            ret += "assert(false); //bad type\n";
                        }
                    }
                    else
                    {
                        ret += "assert(false); //does not exist\n";
                    }

                    ret += "}\n";
                }

                for(int i=0; i < (int)ft_expect.params.size(); i++)
                {
                    stack_offset.pop_back();
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

                ASSERT_SAME_TYPES(get_variable_name(first), get_variable_name(second));

                ///next = decider ? first : second;
                ret += auto_push(get_variable_name(decider) + " ? " + get_variable_name(first) + " : " + get_variable_name(second) + ";\n", stack_offset);
                ASSERT_VALID_TYPE(get_variable_name(stack_offset.peek_back()));

                break;
            }

            ///get_local
            case 0x20:
            {
                types::localidx idx = std::get<types::localidx>(is.dat);

                ret += auto_push(get_local_name((uint32_t)idx) + ";\n", stack_offset);
                ASSERT_VALID_TYPE(get_variable_name(stack_offset.peek_back()));

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
                ASSERT_VALID_TYPE(get_variable_name(stack_offset.peek_back()));

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

            ///mem.size
            case 0x3F:
            {
                minst.memaddrs.check(0);
                runtime::memaddr maddr = minst.memaddrs[0];

                int next_val = stack_offset.get_next();

                ret += "i32 " + get_variable_name(next_val) + " = mem_" + std::to_string((uint32_t)maddr) + ".size() / " + std::to_string(runtime::page_size) + ";\n";

                break;
            }

            ///mem.grow
            case 0x40:
            {
                minst.memaddrs.check(0);
                runtime::memaddr maddr = minst.memaddrs[0];

                std::string mem = "mem_" + std::to_string((uint32_t)maddr);

                int pages_variable = stack_offset.pop_back();
                int return_value = stack_offset.get_next();

                int temp_var = stack_offset.get_temporary();
                int old_size = stack_offset.get_temporary();

                ret += "i32 " + get_variable_name(return_value) + " = 0;\n";

                ret += "i32 " + get_variable_name(temp_var) + " = (" + mem + ".size() / " + std::to_string(runtime::page_size) + ") + " + get_variable_name(pages_variable) + ";\n";
                ret += "i32 " + get_variable_name(old_size) + " = " + mem + ".size() / " + std::to_string(runtime::page_size) + ";\n";

                ret += "if(" + get_variable_name(temp_var) + " * " + std::to_string(runtime::page_size) + " >= " + std::to_string(runtime::sandbox_mem_cap) + "){" + get_variable_name(return_value) + " = -1;}";
                ret += "if(" + get_variable_name(temp_var) + " * " + std::to_string(runtime::page_size) + " < " + std::to_string(runtime::sandbox_mem_cap) + "){" + mem + ".resize(" + get_variable_name(temp_var) + " * " + std::to_string(runtime::page_size) + "+1); " + get_variable_name(return_value) + " = " + get_variable_name(old_size) + ";}";

                break;
            }


            case 0x41:
                C_PUSH_CONSTANT(types::i32);
            case 0x42:
                C_PUSH_CONSTANT(types::i64);
            case 0x43:
                C_PUSH_CONSTANT(types::f32);
            case 0x44:
                C_PUSH_CONSTANT(types::f64);

            ///func, input type, output type
            case 0x45:
                C_POPAT(c_ieqz<uint32_t>, types::i32, types::i32);
            case 0x46:
                C_POPBT(c_eq<uint32_t>, types::i32, types::i32);
            case 0x47:
                C_POPBT(c_ne<uint32_t>, types::i32, types::i32);
            case 0x48:
                C_POPBT(c_ilt_s<int32_t>, types::i32, types::i32);
            case 0x49:
                C_POPBT(c_ilt_u<uint32_t>, types::i32, types::i32);
            case 0x4A:
                C_POPBT(c_igt_s<int32_t>, types::i32, types::i32);
            case 0x4B:
                C_POPBT(c_igt_u<uint32_t>, types::i32, types::i32);
            case 0x4C:
                C_POPBT(c_ile_s<int32_t>, types::i32, types::i32);
            case 0x4D:
                C_POPBT(c_ile_u<uint32_t>, types::i32, types::i32);
            case 0x4E:
                C_POPBT(c_ige_s<int32_t>, types::i32, types::i32);
            case 0x4F:
                C_POPBT(c_ige_u<uint32_t>, types::i32, types::i32);

            case 0x50:
                C_POPAT(c_ieqz<uint64_t>, types::i64, types::i32);
            case 0x51:
                C_POPBT(c_eq<uint64_t>, types::i64, types::i32);
            case 0x52:
                C_POPBT(c_ne<uint64_t>, types::i64, types::i32);
            case 0x53:
                C_POPBT(c_ilt_s<int64_t>, types::i64, types::i32);
            case 0x54:
                C_POPBT(c_ilt_u<uint64_t>, types::i64, types::i32);
            case 0x55:
                C_POPBT(c_igt_s<int64_t>, types::i64, types::i32);
            case 0x56:
                C_POPBT(c_igt_u<uint64_t>, types::i64, types::i32);
            case 0x57:
                C_POPBT(c_ile_s<int64_t>, types::i64, types::i32);
            case 0x58:
                C_POPBT(c_ile_u<uint64_t>, types::i64, types::i32);
            case 0x59:
                C_POPBT(c_ige_s<int64_t>, types::i64, types::i32);
            case 0x5A:
                C_POPBT(c_ige_u<uint64_t>, types::i64, types::i32);


            case 0x5B:
                C_POPBT(c_eq<float>, types::f32, types::i32);
            case 0x5C:
                C_POPBT(c_ne<float>, types::f32, types::i32);
            case 0x5D:
                C_POPBT(c_flt<float>, types::f32, types::i32);
            case 0x5E:
                C_POPBT(c_fgt<float>, types::f32, types::i32);
            case 0x5F:
                C_POPBT(c_fle<float>, types::f32, types::i32);
            case 0x60:
                C_POPBT(c_fge<float>, types::f32, types::i32);

            case 0x61:
                C_POPBT(c_eq<double>, types::f64, types::i32);
            case 0x62:
                C_POPBT(c_ne<double>, types::f64, types::i32);
            case 0x63:
                C_POPBT(c_flt<double>, types::f64, types::i32);
            case 0x64:
                C_POPBT(c_fgt<double>, types::f64, types::i32);
            case 0x65:
                C_POPBT(c_fle<double>, types::f64, types::i32);
            case 0x66:
                C_POPBT(c_fge<double>, types::f64, types::i32);

            case 0x67:
                C_POPAT(c_iclz<uint32_t>, types::i32, types::i32);
            case 0x68:
                C_POPAT(c_ictz<uint32_t>, types::i32, types::i32);
            case 0x69:
                C_POPAT(c_ipopcnt<uint32_t>, types::i32, types::i32);
            case 0x6A:
                C_POPBT(c_add<uint32_t>, types::i32, types::i32);
            case 0x6B:
                C_POPBT(c_sub<uint32_t>, types::i32, types::i32);
            case 0x6C:
                C_POPBT(c_mul<uint32_t>, types::i32, types::i32);
            case 0x6D:
                C_POPBT_RAW(c_idiv<int32_t>, types::i32, types::i32);
            case 0x6E:
                C_POPBT_RAW(c_idiv<uint32_t>, types::i32, types::i32);
            case 0x6F:
                C_POPBT_RAW(c_remi<int32_t>, types::i32, types::i32);
            case 0x70:
                C_POPBT_RAW(c_remi<uint32_t>, types::i32, types::i32);
            case 0x71:
                C_POPBT(c_iand<uint32_t>, types::i32, types::i32);
            case 0x72:
                C_POPBT(c_ior<uint32_t>, types::i32, types::i32);
            case 0x73:
                C_POPBT(c_ixor<uint32_t>, types::i32, types::i32);
            case 0x74:
                C_POPBT(c_ishl<uint32_t>, types::i32, types::i32);
            case 0x75:
                C_POPBT(c_ishr_s<int32_t>, types::i32, types::i32);
            case 0x76:
                C_POPBT(c_ishr_u<uint32_t>, types::i32, types::i32);
            case 0x77:
                C_POPBT(c_irotl<uint32_t>, types::i32, types::i32);
            case 0x78:
                C_POPBT(c_irotr<uint32_t>, types::i32, types::i32);


            case 0x79:
                C_POPAT(c_iclz<uint64_t>, types::i64, types::i64);
            case 0x7A:
                C_POPAT(c_ictz<uint64_t>, types::i64, types::i64);
            case 0x7B:
                C_POPAT(c_ipopcnt<uint64_t>, types::i64, types::i64);
            case 0x7C:
                C_POPBT(c_add<uint64_t>, types::i64, types::i64);
            case 0x7D:
                C_POPBT(c_sub<uint64_t>, types::i64, types::i64);
            case 0x7E:
                C_POPBT(c_mul<uint64_t>, types::i64, types::i64);
            case 0x7F:
                C_POPBT_RAW(c_idiv<int64_t>, types::i64, types::i64);
            case 0x80:
                C_POPBT_RAW(c_idiv<uint64_t>, types::i64, types::i64);
            case 0x81:
                C_POPBT_RAW(c_remi<int64_t>, types::i64, types::i64);
            case 0x82:
                C_POPBT_RAW(c_remi<uint64_t>, types::i64, types::i64);
            case 0x83:
                C_POPBT(c_iand<uint64_t>, types::i64, types::i64);
            case 0x84:
                C_POPBT(c_ior<uint64_t>, types::i64, types::i64);
            case 0x85:
                C_POPBT(c_ixor<uint64_t>, types::i64, types::i64);
            case 0x86:
                C_POPBT(c_ishl<uint64_t>, types::i64, types::i64);
            case 0x87:
                C_POPBT(c_ishr_s<int64_t>, types::i64, types::i64);
            case 0x88:
                C_POPBT(c_ishr_u<uint64_t>, types::i64, types::i64);
            case 0x89:
                C_POPBT(c_irotl<uint64_t>, types::i64, types::i64);
            case 0x8A:
                C_POPBT(c_irotr<uint64_t>, types::i64, types::i64);


            case 0x8B:
                C_POPAT(c_absf<float>, types::f32, types::f32);
            case 0x8C:
                C_POPAT(c_fneg<float>, types::f32, types::f32);
            case 0x8D:
                C_POPAT(c_fceil<float>, types::f32, types::f32);
            case 0x8E:
                C_POPAT(c_ffloor<float>, types::f32, types::f32);
            case 0x8F:
                C_POPAT(c_ftrunc<float>, types::f32, types::f32);
            case 0x90:
                C_POPAT(c_fnearest<float>, types::f32, types::f32);
            case 0x91:
                C_POPAT(c_fsqrt<float>, types::f32, types::f32);
            case 0x92:
                C_POPBT(c_add<float>, types::f32, types::f32);
            case 0x93:
                C_POPBT(c_sub<float>, types::f32, types::f32);
            case 0x94:
                C_POPBT(c_mul<float>, types::f32, types::f32);
            case 0x95:
                C_POPBT(c_fdiv<float>, types::f32, types::f32);
            case 0x96:
                C_POPBT(c_fmin<float>, types::f32, types::f32);
            case 0x97:
                C_POPBT(c_fmax<float>, types::f32, types::f32);
            case 0x98:
                C_POPBT(c_fcopysign<float>, types::f32, types::f32);

            ///these functions are all template parameter format
            ///<dest, src>
            ///so trunc_s takes the argument as a float
            ///and then converts it to an int32_t
            case 0xA7:
                C_POPAT((c_wrap<uint32_t, 32>), types::i64, types::i32);
            case 0xA8:
                C_POPAT((c_trunc_s<int32_t, float>), types::f32, types::i32);
            case 0xA9:
                C_POPAT((c_trunc_u<uint32_t, float>), types::f32, types::i32);
            case 0xAA:
                C_POPAT((c_trunc_s<int32_t, double>), types::f64, types::i32);
            case 0xAB:
                C_POPAT((c_trunc_u<uint32_t, double>), types::f64, types::i32);
            case 0xAC:
                C_POPAT((c_extend_s<int64_t, int32_t>), types::i32, types::i32);
            case 0xAD:
                C_POPAT((c_extend_u<uint64_t, uint32_t>), types::i32, types::i64);
            case 0xAE:
                C_POPAT((c_trunc_s<int64_t, float>), types::f32, types::i64);
            case 0xAF:
                C_POPAT((c_trunc_u<uint64_t, float>), types::f32, types::i64);
            case 0xB0:
                C_POPAT((c_trunc_s<int64_t, double>), types::f64, types::i64);
            case 0xB1:
                C_POPAT((c_trunc_u<uint64_t, double>), types::f64, types::i64);
            case 0xB2:
                C_POPAT((c_convert_s<float, int32_t>), types::i32, types::f32);
            case 0xB3:
                C_POPAT((c_convert_u<float, uint32_t>), types::i32, types::f32);
            case 0xB4:
                C_POPAT((c_convert_s<float, int64_t>), types::i64, types::f32);
            case 0xB5:
                C_POPAT((c_convert_u<float, uint64_t>), types::i64, types::f32);
            case 0xB6:
                C_POPAT((c_demote<float, double>), types::f64, types::f32);
            case 0xB7:
                C_POPAT((c_convert_s<double, int32_t>), types::i32, types::f64);
            case 0xB8:
                C_POPAT((c_convert_u<double, uint32_t>), types::i32, types::f64);
            case 0xB9:
                C_POPAT((c_convert_s<double, int64_t>), types::i64, types::f64);
            case 0xBA:
                C_POPAT((c_convert_u<double, uint64_t>), types::i64, types::f64);
            case 0xBB:
                C_POPAT((c_promote<double, float>), types::f32, types::f64);
            case 0xBC:
                C_POPAT((c_reinterpret<uint32_t, float>), types::f32, types::i32);
            case 0xBD:
                C_POPAT((c_reinterpret<uint64_t, double>), types::f64, types::i32);
            case 0xBE:
                C_POPAT((c_reinterpret<float, uint32_t>), types::i32, types::f32);
            case 0xBF:
                C_POPAT((c_reinterpret<double, uint64_t>), types::i64, types::f64);


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

    int current_locals_end = ftype.params.size();

    std::string function_body = sig + " {\n";

    int return_arity = ftype.results.size() > 0;

    //function_body += "int return_arity = " + std::to_string(return_arity) + ";\n";
    //function_body += "int current_arity = 0;\n";
    //function_body += "int backup_arity = 0;\n";
    function_body += "int abort_stack = 0;\n";

    //function_body += "do {\n";

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

    ///the second check here is either because my code is broken with the stack and it'll bite me in the arse
    ///or, its because something weird's going on with wasi's stack when it generates returns
    if(return_arity > 0 && soffset.stk.size() > 0)
        return function_body + "return " + get_variable_name(soffset.pop_back()) + ";\n}\n";
    else
        return function_body + "}\n";
}

std::string compile_top_level(runtime::store& s, runtime::moduleinst& minst)
{
    ///need to inject memory

    std::string res =
    R"(
#include <vector>
#include <cstdint>
#include <typeinfo>
#include <type_traits>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h> ///not necessary
#include <iostream> ///not necessary

using i32 = uint32_t;
using i64 = uint64_t;
using f32 = float;
using f64 = double;
using empty = void;

)";

    bool has_main = false;

    for(runtime::exportinst& einst : minst.exports)
    {
        if(einst.name == "main")
        {
            has_main = true;
            break;
        }
    }

    for(int i=0; i < (int)s.mems.size(); i++)
    {
        runtime::meminst& inst = s.mems[i];

        uint64_t mem_size = inst.dat.size();

        res += "std::vector<char> mem_" + std::to_string(i) + "(" + std::to_string(mem_size) + ");\n";
    }

    res += "\n#include \"wasi_impl.hpp\"\n";

    for(runtime::globaladdr addr : minst.globaladdrs)
    {
        uint32_t idx = (uint32_t)addr;
        runtime::globalinst& glob = s.globals[(uint32_t)addr];

        res += glob.val.friendly() + " " + get_global_name(idx) + " = (" + glob.val.friendly() + ")" + glob.val.friendly_val() + ";\n";
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

    if(!has_main)
    {
        res += "int main(){proc_exit(__original_main());}\n\n";
    }

    return res;
}


std::string get_as_c_program(wasm_binary_data& bin)
{
    /*for(runtime::exportinst& einst : bin.m_minst->exports)
    {
        if(einst.name == "main")
        {
            return compile_top_level(bin.s, std::get<runtime::funcaddr>(einst.value.val), *bin.m_minst, {});
        }
    }

    for(runtime::exportinst& einst : bin.m_minst->exports)
    {
        if(einst.name == "_start")
        {
            return compile_top_level(bin.s, std::get<runtime::funcaddr>(einst.value.val), *bin.m_minst, {});
        }
    }

    for(runtime::exportinst& einst : bin.m_minst->exports)
    {
        if(einst.name == "__original_main")
        {
            return compile_top_level(bin.s, std::get<runtime::funcaddr>(einst.value.val), *bin.m_minst, {});
        }
    }

    throw std::runtime_error("Did not find main");*/

    return compile_top_level(bin.s, *bin.m_minst);
}
