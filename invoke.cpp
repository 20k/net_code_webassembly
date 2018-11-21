#include "invoke.hpp"
#include "runtime_types.hpp"
#include "basic_ops.hpp"

template<typename T>
void push(const T& t, full_stack& full)
{
    runtime::value val;
    val.set(t);

    full.push_values(val);
}

#define PUSH(x) return push(x, full); break;
#define POP() full.pop_back()

#define POPA(x) PUSH(full.pop_back().apply(x))

inline
void do_op(const uint8_t& which, full_stack& full)
{
    switch(which)
    {
        case 0xA7:
            POPA((wrap<uint32_t, 64>));
    }
}

void eval_expr(const types::expr& exp, full_stack& full)
{
    ///thisll break until at minimum we pop the values off the stack
    ///but obviously we actually wanna parse stuff

    int len = exp.i.size();

    for(int i=0; i < len; i++)
    {
        const types::instr& ins = exp.i[i];


    }
}

void invoke_intl(runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    /*if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch");*/

    int num_args = ftype.params.size();

    if(std::holds_alternative<runtime::webasm_func>(finst.funct))
    {
        runtime::webasm_func fnc = std::get<runtime::webasm_func>(finst.funct);

        types::vec<runtime::value> popped = full.pop_num_vals(num_args);

        types::vec<types::local> local_types = fnc.funct.fnc.locals;
        types::expr expression = fnc.funct.fnc.e;

        types::vec<runtime::value> local_zeroes;

        for(const types::local& loc : local_types)
        {
            for(uint32_t i=0; i < (uint32_t)loc.n; i++)
            {
                runtime::value val;
                val.from_valtype(loc.type);

                local_zeroes.push_back(val);
            }
        }

        frame fr;
        ///SUPER BAD CODE ALERT
        fr.inst = &minst;

        fr.locals = popped;
        fr.locals.append(local_zeroes);

        activation activate;
        activate.return_arity = types::s32{ftype.results.size()};

        full.push_activation(activate);

        eval_expr(expression, full);

        ///not sure i need to refetch this activation here
        activation& current = full.get_current();

        types::vec<runtime::value> found = full.pop_num_vals((int32_t)current.return_arity);

        full.ensure_activation();
        full.pop_back();

        for(auto& i : found)
            full.push_values(i);

        ///carry on instruction stream after the call
        ///as you do
    }
    else
    {
        throw std::runtime_error("Bad function invocation type");
    }
}

void runtime::store::invoke(const runtime::funcaddr& address, runtime::moduleinst& minst, const types::vec<runtime::value>& vals)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = funcs[adr];

    types::functype ftype = finst.type;

    if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch");

    full_stack full;

    for(auto& val : vals)
    {
        full.push_values(val);
    }

    invoke_intl(*this, full, address, minst);

    ///pop val from stack

    /*if(ftype.results.size() > 0)
    {
        auto res = full.pop_num_vals(1);
    }*/
}
