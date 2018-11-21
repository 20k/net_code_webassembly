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

//#define PUSH(x) return push(x, full); break;
#define POP() full.pop_back()

#define POPA(x) return push(full.pop_back().apply(x), full); break;
#define POPB(x) {auto a1 = full.pop_back(); auto a2 = full.pop_back(); push(runtime::apply(x, a1, a2), full); break; return;}

inline
void do_op(const uint8_t& which, full_stack& full)
{
    switch(which)
    {


        case 0x67:
            POPA(iclz<uint32_t>);
        case 0x68:
            POPA(ictz<uint32_t>);
        case 0x69:
            POPA(ipopcnt<uint32_t>);
        case 0x6A:
            POPB(add<uint32_t>);
        case 0x6B:
            POPB(sub<uint32_t>);
        case 0x6C:
            POPB(mul<uint32_t>);
        case 0x6D:
            POPB(idiv<int32_t>);
        case 0x6E:
            POPB(idiv<uint32_t>);
        case 0x6F:
            POPB(remi<int32_t>);
        case 0x70:
            POPB(remi<uint32_t>);
        case 0x71:
            POPB(iand<uint32_t>);
        case 0x72:
            POPB(ior<uint32_t>);
        case 0x73:
            POPB(ixor<uint32_t>);
        case 0x74:
            POPB(ishl<uint32_t>);
        case 0x75:
            POPB(ishr_s<int32_t>);
        case 0x76:
            POPB(ishr_u<uint32_t>);
        case 0x77:
            POPB(irotl<uint32_t>);
        case 0x78:
            POPB(irotr<uint32_t>);


        case 0x79:
            POPA(iclz<uint64_t>);
        case 0x7A:
            POPA(ictz<uint64_t>);
        case 0x7B:
            POPA(ipopcnt<uint64_t>);
        case 0x7C:
            POPB(add<uint64_t>);
        case 0x7D:
            POPB(sub<uint64_t>);
        case 0x7E:
            POPB(mul<uint64_t>);
        case 0x7F:
            POPB(idiv<int64_t>);
        case 0x80:
            POPB(idiv<uint64_t>);
        case 0x81:
            POPB(remi<int64_t>);
        case 0x82:
            POPB(remi<uint64_t>);
        case 0x83:
            POPB(iand<uint64_t>);
        case 0x84:
            POPB(ior<uint64_t>);
        case 0x85:
            POPB(ixor<uint64_t>);
        case 0x86:
            POPB(ishl<uint64_t>);
        case 0x87:
            POPB(ishr_s<int64_t>);
        case 0x88:
            POPB(ishr_u<uint64_t>);
        case 0x89:
            POPB(irotl<uint64_t>);
        case 0x8A:
            POPB(irotr<uint64_t>);

        case 0x8B:
            POPA(absf<float>);
        case 0x8C:
            POPA(fneg<float>);
        case 0x8D:
            POPA(fceil<float>);
        case 0x8E:
            POPA(ffloor<float>);
        case 0x8F:
            POPA(ftrunc<float>);
        case 0x90:
            POPA(fnearest<float>);
        case 0x91:
            POPA(fsqrt<float>);
        case 0x92:
            POPB((add<float>));
        case 0x93:
            POPB(sub<float>);
        case 0x94:
            POPB(mul<float>);
        case 0x95:
            POPB(fdiv<float>);
        case 0x96:
            POPB(fmin<float>);
        case 0x97:
            POPB(fmax<float>);
        case 0x98:
            POPB(fcopysign<float>);

        case 0x99:
            POPA(absf<double>);
        case 0x9A:
            POPA(fneg<double>);
        case 0x9B:
            POPA(fceil<double>);
        case 0x9C:
            POPA(ffloor<double>);
        case 0x9D:
            POPA(ftrunc<double>);
        case 0x9E:
            POPA(fnearest<double>);
        case 0x9F:
            POPA(fsqrt<double>);
        case 0xA0:
            POPB((add<double>));
        case 0xA1:
            POPB(sub<double>);
        case 0xA2:
            POPB(mul<double>);
        case 0xA3:
            POPB(fdiv<double>);
        case 0xA4:
            POPB(fmin<double>);
        case 0xA5:
            POPB(fmax<double>);
        case 0xA6:
            POPB(fcopysign<double>);

        ///these functions are all template parameter format
        ///<dest, src>
        ///so trunc_s takes the argument as a float
        ///and then converts it to an in32_t
        case 0xA7:
            POPA((wrap<uint32_t, 64>));
        case 0xA8:
            POPA((trunc_s<int32_t, float>));
        case 0xA9:
            POPA((trunc_u<uint32_t, float>));
        case 0xAA:
            POPA((trunc_s<int32_t, double>));
        case 0xAB:
            POPA((trunc_u<uint32_t, double>));
        case 0xAC:
            POPA((extend_s<int64_t, int32_t>));
        case 0xAD:
            POPA((extend_u<uint64_t, int32_t>));
        case 0xAE:
            POPA((trunc_s<int64_t, float>));
        case 0xAF:
            POPA((trunc_u<uint64_t, float>));
        case 0xB0:
            POPA((trunc_s<int64_t, double>));
        case 0xB1:
            POPA((trunc_u<uint64_t, double>));
        case 0xB2:
            POPA((convert_s<float, int32_t>));
        case 0xB3:
            POPA((convert_u<float, uint32_t>));
        case 0xB4:
            POPA((convert_s<float, int64_t>));
        case 0xB5:
            POPA((convert_u<float, uint64_t>));
        case 0xB6:
            POPA((demote<float, double>));
        case 0xB7:
            POPA((convert_s<double, int32_t>));
        case 0xB8:
            POPA((convert_u<double, uint32_t>));
        case 0xB9:
            POPA((convert_s<double, int64_t>));
        case 0xBA:
            POPA((convert_u<double, uint64_t>));
        case 0xBB:
            POPA((promote<double, float>));
        case 0xBC:
            POPA((reinterpret<uint32_t, float>));
        case 0xBD:
            POPA((reinterpret<uint64_t, double>));
        case 0xBE:
            POPA((reinterpret<float, uint32_t>));
        case 0xBF:
            POPA((reinterpret<double, uint64_t>));
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
