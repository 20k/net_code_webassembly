#include "invoke.hpp"
#include "runtime_types.hpp"

template<typename T>
bool bool_op(const T& t)
{
    return t > 0;
}

template<typename T>
T add(const T& v1, const T& v2)
{
    return v1 + v2;
}

template<typename T>
T sub(const T& v1, const T& v2)
{
    return v1 - v2;
}

template<typename T>
T mul(const T& v1, const T& v2)
{
    return v1 * v2;
}

template<typename T>
T divu(const T& v1, const T& v2)
{
    if(v2 == 0)
        throw std::runtime_error("v2 == 0");

    return v1 / v2;
}

template<typename T>
T divf(const T& v1, const T& v2)
{
    return v1 / v2;
}

template<typename T>
T remi(const T& v1, const T& v2)
{
    if(v2 == 0)
        throw std::runtime_error("v2 == 0 in rem");

    return v1 % v2;
}

template<typename T>
T remf(const T& v1, const T& v2)
{
    return fmod(v1, v2);
}

template<typename T>
T iand(const T& v1, const T& v2)
{
    return v1 & v2;
}

template<typename T>
T ior(const T& v1, const T& v2)
{
    return v1 | v2;
}

template<typename T>
T ixor(const T& v1, const T& v2)
{
    return v1 ^ v2;
}

template<typename T>
T ishl(const T& v1, const T& v2)
{
    return v1 << v2;
}

///NEED TO CONVERT TYPES HERE
template<typename T>
T ishr_u(const T& v1, const T& v2)
{
    return v1 >> v2;
}

template<typename T>
T ishr_s(const T& v1, const T& v2)
{
    return v1 >> v2;
}

template<typename T>
T irotl(const T& v1, const T& v2)
{
    return (v1 << v2)|(v1 >> ((sizeof(T)*8) - v2));
}

template<typename T>
T irotr(const T& v1, const T& v2)
{
    return (v1 >> v2)|(v1 << (sizeof(T)*8 - v2));
}

template<typename T>
T iclz(const T& v1)
{
    return __builtin_clz(v1);
}

template<typename T>
T ictz(const T& v1)
{
    return __builtin_ctz(v1);
}

template<typename T>
T ipopcnt(const T& v1)
{
    return __builtin_popcount(v1);
}

template<typename T>
T ieqz(const T& v1)
{
    return v1 == 0;
}

template<typename T>
T ieq(const T& v1, const T& v2)
{
    return v1 == v2;
}

template<typename T>
T ine(const T& v1, const T& v2)
{
    return v1 != v2;
}

template<typename T>
T ilt_u(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
T ilt_s(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
T igt_u(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
T igt_s(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
T ile_u(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
T ile_s(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
T ige_u(const T& v1, const T& v2)
{
    return v1 >= v2;
}

template<typename T>
T ige_s(const T& v1, const T& v2)
{
    return v1 >= v2;
}

void eval_expr(const types::expr& exp, full_stack& full)
{
    ///thisll break until at minimum we pop the values off the stack
    ///but obviously we actually wanna parse stuff



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
