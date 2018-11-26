#include "invoke.hpp"
#include "runtime_types.hpp"
#include "basic_ops.hpp"
#include <iostream>
#include <map>

///all the popa functions know what types they're dealing with, could be a perf boost to avoid apply
//#define POPA(x) {auto a1 = full.pop_back(); push(runtime::apply(x, a1), full); break;}
//#define POPB(x) {auto a2 = full.pop_back(); auto a1 = full.pop_back(); push(runtime::apply(x, a1, a2), full); break;}
//#define POPB(x) {runtime::value a2; runtime::value a1; full.pop_2(a1, a2); push(runtime::apply(x, a1, a2), full); break;}
//#define POPBT(x, y) {auto a2 = full.pop_back(); auto a1 = full.pop_back(); full.push_values(x(std::get<y>(a1.v).val, std::get<y>(a2.v).val)); break;}

#define POPAT(x, y) {auto a1 = full.pop_back(); full.push_values(x(std::get<y>(a1.v).val)); break;}
#define POPBT(x, y) {runtime::value a2; runtime::value a1; full.pop_2(a1, a2); full.push_values(x(std::get<y>(a1.v).val, std::get<y>(a2.v).val)); break;}

#define PUSH_CONSTANT(xtype)\
        { \
            /*lg::log("loaded constant ", cst.val);*/ \
            full.push_values(std::get<xtype>(is.dat)); \
            break; \
        }

#define MEM_LOAD(x, y) mem_load<x, y>(s, std::get<types::memarg>(is.dat), full, activate); break;
#define MEM_STORE(x, y) mem_store<x, y>(s, std::get<types::memarg>(is.dat), full, activate); break;

#define MEM_SIZE() memory_size(full, s, activate); break;
#define MEM_GROW() memory_grow(full, s, activate); break;

#define INVOKE_LOCAL(f) f(full, std::get<types::localidx>(is.dat), activate); break;
#define INVOKE_GLOBAL(f) f(s, full, std::get<types::globalidx>(is.dat), activate); break;

///maybe the trick is that labels don't really exist
///and it just carries on from that one

struct info_stack;

struct context
{
    int abort_stack = 0;
    //int continuation = 0;
    int expression_counter = 0;
    //bool needs_cont_jump = false;
    int current_arity = 0;

    bool frame_abort = false;

    types::vec<info_stack>& istack;

    context(types::vec<info_stack>& st) : istack(st)
    {

    }

    //types::vec<runtime::value> capture_vals;

    runtime::value capture_val;
    int capture_arity = 0;

    bool break_op_loop()
    {
        return abort_stack > 0 || frame_abort;
    }
};

//#define DEBUGGING

void eval_with_label(context& ctx, runtime::store& s, const label& l, const types::vec<types::instr>& exp, full_stack& full, activation& activate);
types::vec<runtime::value> invoke_intl(context& ctx, runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst);

inline
void fjump(context& ctx, types::labelidx lidx, full_stack& full)
{
    //const label& l = full.get_current_label();

    //int arity = l.btype.arity();

    int arity = ctx.current_arity;

    ///I think the below statement is correct, and if it is it has better performance
    //int arity = full.current_stack_size()

    /*label& olab = full.get_label_of_offset((uint32_t)lidx);

    //lg::log("ctype ", std::to_string(olab.continuation));

    ctx.continuation = olab.continuation;*/

    if(arity == 1)
        ctx.capture_val = full.pop_back();

    ctx.capture_arity = arity;

    //ctx.capture_vals = full.pop_num_vals(arity);

    ctx.abort_stack = (uint32_t)lidx + 1;
    //ctx.needs_cont_jump = true;
}

/*void fjump_up_frame(context& ctx, full_stack& full)
{
    activation& activate = full.get_current();

    int arity = (int32_t)activate.return_arity;

    if(arity == 1)
        ctx.capture_val = full.pop_back();

    ctx.capture_arity = arity;

    //ctx.capture_vals = full.pop_num_vals(arity);
    ctx.frame_abort = true;
}*/

void fjump_up_frame_with(context& ctx, full_stack& full, activation& activate)
{
    int arity = (int32_t)activate.return_arity;

    if(arity == 1)
        ctx.capture_val = full.pop_back();

    ctx.capture_arity = arity;

    //ctx.capture_vals = full.pop_num_vals(arity);
    ctx.frame_abort = true;
}

//#define PERF

struct binary_profiler
{
    std::map<uint8_t, int> i_count;

    void add(uint8_t in)
    {
        i_count[in]++;
    }

    ~binary_profiler()
    {
        //int sum = 0;

        for(auto& i : i_count)
        {
            std::cout << std::hex << "0x" << (uint32_t)i.first << " " << i.second << std::dec << ", ";
        }

        std::cout << std::endl;
    }
};

#ifdef DEBUGGING
struct nest_counter
{
    context& ctx;

    nest_counter(context& lctx) : ctx(lctx)
    {
        ctx.expression_counter++;
    }

    ~nest_counter()
    {
        ctx.expression_counter--;
    }
};

struct stack_counter
{
    full_stack& stk;

    stack_counter(full_stack& lstk) : stk(lstk)
    {

    }

    ~stack_counter()
    {
        lg::log("s ", stk.value_stack_size());
    }
};
#endif // DEBUGGING

///so duktape takes about 330ms
///and we take about 2000ms
///now we take about 1100
///or in the recursive version, seemingly about 980

///dump value of globals and follow everything through to see
///if its the leadup to strlen which is incorrect
//__attribute__((optimize("unroll-loops")))
void eval_expr(context& ctx, runtime::store& s, const types::vec<types::instr>& exp, full_stack& full, activation& activate)
{
    ///thisll break until at minimum we pop the values off the stack
    ///but obviously we actually wanna parse stuff

    #ifdef PERF
    binary_profiler prof;
    #endif // PERF

    #ifdef DEBUGGING
    nest_counter nest(ctx);
    #endif // DEBUGGING

    int len = exp.size();

    for(int ilen=0; ilen < len; ilen++)
    {
        #ifdef DEBUGGING
        //stack_counter stk(full);
        #endif // DEBUGGING

        const types::instr& is = exp[ilen];

        uint8_t which = is.which;

        #ifdef PERF
        prof.add(which);
        #endif // PERF


        #ifdef DEBUGGING
        std::cout << "0x" << std::hex << (uint32_t)which << std::dec << " " << ctx.expression_counter << std::endl;
        #endif // DEBUGGING

        /*lg::log("0x");
        lg::log_hex_noline(which);
        lg::log(", ");*/

        ///good lord this is tedious
        switch(which)
        {
            case 0x00:
                throw std::runtime_error("unreachable");
                break;

            case 0x01:
                break;

            case 0x02:
            {
                const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                label l;
                l.btype = sbd.btype;
                l.continuation = 1;

                eval_with_label(ctx, s, l, sbd.first, full, activate);

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x03:
            {
                const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                label l;
                l.btype = sbd.btype;
                l.continuation = 2;

                if(l.btype.arity() != 0)
                    throw std::runtime_error("Wrong arity?");

                eval_with_label(ctx, s, l, sbd.first, full, activate);

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x04:
            {
                const types::double_branch_data& dbd = std::get<types::double_branch_data>(is.dat);

                label l;
                l.btype = dbd.btype;
                l.continuation = 3;

                runtime::value val = full.pop_back();

                if(!val.is_i32())
                    throw std::runtime_error("0x04 if/else expected i32");

                types::i32 type = std::get<types::i32>(val.v);

                uint32_t c = type.val;

                if(c != 0)
                {
                    eval_with_label(ctx, s, l, dbd.first, full, activate);
                }
                else
                {
                    eval_with_label(ctx, s, l, dbd.second, full, activate);
                }

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x0C:
            {
                types::labelidx lidx = std::get<types::labelidx>(is.dat);

                uint32_t idx = (uint32_t)lidx;

                /*if((uint32_t)full.num_labels() < idx + 1)
                {
                    throw std::runtime_error("not enough labels");
                }*/

                fjump(ctx, lidx, full);

                if(ctx.break_op_loop())
                    ilen = len;

                //lg::log("hit br ", std::to_string(idx));

                break;
            }

            case 0x0D:
            {
                runtime::value val = full.pop_back();

                if(!val.is_i32())
                    throw std::runtime_error("expected i32 in 0x0D");

                //std::cout << "good " << std::holds_alternative<types::i32>(val.v) << std::endl;

                types::i32 type = std::get<types::i32>(val.v);

                //std::cout << "post good\n";

                #ifdef DEBUGGING
                lg::log("hit br_if");
                #endif // DEBUGGING

                if((uint32_t)type != 0)
                {
                    types::labelidx lidx = std::get<types::labelidx>(is.dat);

                    fjump(ctx, lidx, full);

                    uint32_t idx = (uint32_t)lidx;

                    /*if((uint32_t)full.num_labels() < idx + 1)
                    {
                        throw std::runtime_error("not enough labels");
                    }*/

                    if(ctx.break_op_loop())
                        ilen = len;

                    #ifdef DEBUGGING
                    lg::log("took branch to ", std::to_string(idx));
                    #endif // DEBUGGING

                    //std::cout << "hit br_if\n";
                }

                break;
            }

            case 0x0E:
            {
                const types::br_table_data& br_td = std::get<types::br_table_data>(is.dat);

                runtime::value top_val = full.pop_back();

                if(!top_val.is_i32())
                    throw std::runtime_error("br_td should have been i32");

                types::i32 val = std::get<types::i32>(top_val.v);

                uint32_t idx = (uint32_t)val;

                if(idx < (uint32_t)br_td.labels.size())
                {
                    types::labelidx lidx = br_td.labels[idx];

                    fjump(ctx, lidx, full);
                }
                else
                {
                    fjump(ctx, br_td.fin, full);
                }

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x0F:
            {
                #ifdef DEBUGGING
                lg::log("Stk size ", full.value_stack_size());
                lg::log("THIS IS RETURNING HELLO ", full.peek_back().value_or(runtime::value{types::i32{543}}).friendly_val());
                #endif // DEBUGGING

                fjump_up_frame_with(ctx, full, activate);

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x10:
            {
                types::funcidx fidx = std::get<types::funcidx>(is.dat);

                uint32_t idx = (uint32_t)fidx;

                if(idx >= (uint32_t)activate.f.inst->funcaddrs.size())
                    throw std::runtime_error("Bad fidx in 0x10");

                #ifdef DEBUGGING
                lg::log("calling hi there, value stack size is ", full.value_stack_size());
                #endif // DEBUGGING

                invoke_intl(ctx, s, full, activate.f.inst->funcaddrs[idx], *activate.f.inst);

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x11:
            {
                ///alright indirect calls
                types::funcidx found_fidx = std::get<types::funcidx>(is.dat);

                runtime::moduleinst* inst = activate.f.inst;

                if(inst->tableaddrs.size() < 1)
                    throw std::runtime_error("Failed table addr < 1 in indirect call");

                runtime::tableaddr taddr = inst->tableaddrs[0];

                uint32_t tidx = (uint32_t)taddr;

                if(tidx >= (uint32_t)s.tables.size())
                    throw std::runtime_error("Bad tidx in indirect call");

                runtime::tableinst& tinst = s.tables[tidx];

                uint32_t fidx = (uint32_t)found_fidx;

                if(fidx >= (uint32_t)inst->typel.size())
                    throw std::runtime_error("Bad fidx");

                types::functype ft_expect = inst->typel[fidx];


                runtime::value val = full.pop_back();

                if(!val.is_i32())
                    throw std::runtime_error("Not i32 in call indirect");

                types::i32 i_call = std::get<types::i32>(val.v);

                uint32_t i_call_idx = (uint32_t)i_call;

                if(i_call_idx >= (uint32_t)tinst.elem.size())
                    throw std::runtime_error("Bad i_call in indirect call");

                if(!tinst.elem[i_call_idx].addr.has_value())
                    throw std::runtime_error("No faddr in indirect call");

                runtime::funcaddr runtime_addr = tinst.elem[i_call_idx].addr.value();

                uint32_t runtime_idx = (uint32_t)runtime_addr;

                if(runtime_idx >= (uint32_t)s.funcs.size())
                    throw std::runtime_error("Runtime idx oob (validation)");

                runtime::funcinst& finst = s.funcs[runtime_idx];

                types::functype ft_actual = finst.type;

                if(!types::funcs_equal(ft_actual, ft_expect))
                    throw std::runtime_error("Expected and actual types of funcs differ");

                invoke_intl(ctx, s, full, runtime_addr, *inst);

                if(ctx.break_op_loop())
                    ilen = len;

                break;
            }

            case 0x1A:
            {
                full.pop_back();

                break;
            }

            case 0x1B:
            {
                select(full);

                break;
            }

            case 0x20:
                INVOKE_LOCAL(get_local);
            case 0x21:
                INVOKE_LOCAL(set_local);
            case 0x22:
                INVOKE_LOCAL(tee_local);
            case 0x23:
                INVOKE_GLOBAL(get_global);
            case 0x24:
                INVOKE_GLOBAL(set_global);

            case 0x28:
                MEM_LOAD(uint32_t, uint32_t);
            case 0x29:
                MEM_LOAD(uint64_t, uint64_t);
            case 0x2A:
                MEM_LOAD(float, float);
            case 0x2B:
                MEM_LOAD(double, double);
            case 0x2C:
                MEM_LOAD(int32_t, int8_t);
            case 0x2D:
                MEM_LOAD(uint32_t, uint8_t);
            case 0x2E:
                MEM_LOAD(int32_t, int16_t);
            case 0x2F:
                MEM_LOAD(uint32_t, uint16_t);
            case 0x30:
                MEM_LOAD(int64_t, int8_t);
            case 0x31:
                MEM_LOAD(uint64_t, uint8_t);
            case 0x32:
                MEM_LOAD(int64_t, int16_t);
            case 0x33:
                MEM_LOAD(uint64_t, uint16_t);
            case 0x34:
                MEM_LOAD(int64_t, int32_t);
            case 0x35:
                MEM_LOAD(uint64_t, uint32_t);

            case 0x36:
                MEM_STORE(uint32_t, 4);
            case 0x37:
                MEM_STORE(uint64_t, 4);
            case 0x38:
                MEM_STORE(float, 4);
            case 0x39:
                MEM_STORE(double, 4);
            case 0x3A:
                MEM_STORE(uint32_t, 1);
            case 0x3B:
                MEM_STORE(uint32_t, 2);
            case 0x3C:
                MEM_STORE(uint64_t, 1);
            case 0x3D:
                MEM_STORE(uint64_t, 2);
            case 0x3E:
                MEM_STORE(uint64_t, 4);

            case 0x3F:
                MEM_SIZE();
            case 0x40:
                MEM_GROW();

            ///TODO size and grow

            case 0x41:
                PUSH_CONSTANT(types::i32);
            case 0x42:
                PUSH_CONSTANT(types::i64);
            case 0x43:
                PUSH_CONSTANT(types::f32);
            case 0x44:
                PUSH_CONSTANT(types::f64);

            case 0x45:
                POPAT(ieqz<uint32_t>, types::i32);
            case 0x46:
                POPBT(eq<uint32_t>, types::i32);
            case 0x47:
                POPBT(ne<uint32_t>, types::i32);
            case 0x48:
                POPBT(ilt_s<int32_t>, types::i32);
            case 0x49:
                POPBT(ilt_u<uint32_t>, types::i32);
            case 0x4A:
                POPBT(igt_s<int32_t>, types::i32);
            case 0x4B:
                POPBT(igt_u<uint32_t>, types::i32);
            case 0x4C:
                POPBT(ile_s<int32_t>, types::i32);
            case 0x4D:
                POPBT(ile_u<uint32_t>, types::i32);
            case 0x4E:
                POPBT(ige_s<int32_t>, types::i32);
            case 0x4F:
                POPBT(ige_u<uint32_t>, types::i32);

            case 0x50:
                POPAT(ieqz<uint64_t>, types::i64);
            case 0x51:
                POPBT(eq<uint64_t>, types::i64);
            case 0x52:
                POPBT(ne<uint64_t>, types::i64);
            case 0x53:
                POPBT(ilt_s<int64_t>, types::i64);
            case 0x54:
                POPBT(ilt_u<uint64_t>, types::i64);
            case 0x55:
                POPBT(igt_s<int64_t>, types::i64);
            case 0x56:
                POPBT(igt_u<uint64_t>, types::i64);
            case 0x57:
                POPBT(ile_s<int64_t>, types::i64);
            case 0x58:
                POPBT(ile_u<uint64_t>, types::i64);
            case 0x59:
                POPBT(ige_s<int64_t>, types::i64);
            case 0x5A:
                POPBT(ige_u<uint64_t>, types::i64);

            case 0x5B:
                POPBT(eq<float>, types::f32);
            case 0x5C:
                POPBT(ne<float>, types::f32);
            case 0x5D:
                POPBT(flt<float>, types::f32);
            case 0x5E:
                POPBT(fgt<float>, types::f32);
            case 0x5F:
                POPBT(fle<float>, types::f32);
            case 0x60:
                POPBT(fge<float>, types::f32);

            case 0x61:
                POPBT(eq<double>, types::f64);
            case 0x62:
                POPBT(ne<double>, types::f64);
            case 0x63:
                POPBT(flt<double>, types::f64);
            case 0x64:
                POPBT(fgt<double>, types::f64);
            case 0x65:
                POPBT(fle<double>, types::f64);
            case 0x66:
                POPBT(fge<double>, types::f64);

            case 0x67:
                POPAT(iclz<uint32_t>, types::i32);
            case 0x68:
                POPAT(ictz<uint32_t>, types::i32);
            case 0x69:
                POPAT(ipopcnt<uint32_t>, types::i32);
            case 0x6A:
                POPBT(add<uint32_t>, types::i32);
            case 0x6B:
                POPBT(sub<uint32_t>, types::i32);
            case 0x6C:
                POPBT(mul<uint32_t>, types::i32);
            case 0x6D:
                POPBT(idiv<int32_t>, types::i32);
            case 0x6E:
                POPBT(idiv<uint32_t>, types::i32);
            case 0x6F:
                POPBT(remi<int32_t>, types::i32);
            case 0x70:
                POPBT(remi<uint32_t>, types::i32);
            case 0x71:
                POPBT(iand<uint32_t>, types::i32);
            case 0x72:
                POPBT(ior<uint32_t>, types::i32);
            case 0x73:
                POPBT(ixor<uint32_t>, types::i32);
            case 0x74:
                POPBT(ishl<uint32_t>, types::i32);
            case 0x75:
                POPBT(ishr_s<int32_t>, types::i32);
            case 0x76:
                POPBT(ishr_u<uint32_t>, types::i32);
            case 0x77:
                POPBT(irotl<uint32_t>, types::i32);
            case 0x78:
                POPBT(irotr<uint32_t>, types::i32);


            case 0x79:
                POPAT(iclz<uint64_t>, types::i64);
            case 0x7A:
                POPAT(ictz<uint64_t>, types::i64);
            case 0x7B:
                POPAT(ipopcnt<uint64_t>, types::i64);
            case 0x7C:
                POPBT(add<uint64_t>, types::i64);
            case 0x7D:
                POPBT(sub<uint64_t>, types::i64);
            case 0x7E:
                POPBT(mul<uint64_t>, types::i64);
            case 0x7F:
                POPBT(idiv<int64_t>, types::i64);
            case 0x80:
                POPBT(idiv<uint64_t>, types::i64);
            case 0x81:
                POPBT(remi<int64_t>, types::i64);
            case 0x82:
                POPBT(remi<uint64_t>, types::i64);
            case 0x83:
                POPBT(iand<uint64_t>, types::i64);
            case 0x84:
                POPBT(ior<uint64_t>, types::i64);
            case 0x85:
                POPBT(ixor<uint64_t>, types::i64);
            case 0x86:
                POPBT(ishl<uint64_t>, types::i64);
            case 0x87:
                POPBT(ishr_s<int64_t>, types::i64);
            case 0x88:
                POPBT(ishr_u<uint64_t>, types::i64);
            case 0x89:
                POPBT(irotl<uint64_t>, types::i64);
            case 0x8A:
                POPBT(irotr<uint64_t>, types::i64);

            case 0x8B:
                POPAT(absf<float>, types::f32);
            case 0x8C:
                POPAT(fneg<float>, types::f32);
            case 0x8D:
                POPAT(fceil<float>, types::f32);
            case 0x8E:
                POPAT(ffloor<float>, types::f32);
            case 0x8F:
                POPAT(ftrunc<float>, types::f32);
            case 0x90:
                POPAT(fnearest<float>, types::f32);
            case 0x91:
                POPAT(fsqrt<float>, types::f32);
            case 0x92:
                POPBT(add<float>, types::f32);
            case 0x93:
                POPBT(sub<float>, types::f32);
            case 0x94:
                POPBT(mul<float>, types::f32);
            case 0x95:
                POPBT(fdiv<float>, types::f32);
            case 0x96:
                POPBT(fmin<float>, types::f32);
            case 0x97:
                POPBT(fmax<float>, types::f32);
            case 0x98:
                POPBT(fcopysign<float>, types::f32);

            case 0x99:
                POPAT(absf<double>, types::f64);
            case 0x9A:
                POPAT(fneg<double>, types::f64);
            case 0x9B:
                POPAT(fceil<double>, types::f64);
            case 0x9C:
                POPAT(ffloor<double>, types::f64);
            case 0x9D:
                POPAT(ftrunc<double>, types::f64);
            case 0x9E:
                POPAT(fnearest<double>, types::f64);
            case 0x9F:
                POPAT(fsqrt<double>, types::f64);
            case 0xA0:
                POPBT(add<double>, types::f64);
            case 0xA1:
                POPBT(sub<double>, types::f64);
            case 0xA2:
                POPBT(mul<double>, types::f64);
            case 0xA3:
                POPBT(fdiv<double>, types::f64);
            case 0xA4:
                POPBT(fmin<double>, types::f64);
            case 0xA5:
                POPBT(fmax<double>, types::f64);
            case 0xA6:
                POPBT(fcopysign<double>, types::f64);

            ///these functions are all template parameter format
            ///<dest, src>
            ///so trunc_s takes the argument as a float
            ///and then converts it to an in32_t
            case 0xA7:
                POPAT((wrap<uint32_t, 64>), types::i32);
            case 0xA8:
                POPAT((trunc_s<int32_t, float>), types::f32);
            case 0xA9:
                POPAT((trunc_u<uint32_t, float>), types::f32);
            case 0xAA:
                POPAT((trunc_s<int32_t, double>), types::f64);
            case 0xAB:
                POPAT((trunc_u<uint32_t, double>), types::f64);
            case 0xAC:
                POPAT((extend_s<int64_t, int32_t>), types::i32);
            case 0xAD:
                POPAT((extend_u<uint64_t, int32_t>), types::i32);
            case 0xAE:
                POPAT((trunc_s<int64_t, float>), types::f32);
            case 0xAF:
                POPAT((trunc_u<uint64_t, float>), types::f32);
            case 0xB0:
                POPAT((trunc_s<int64_t, double>), types::f64);
            case 0xB1:
                POPAT((trunc_u<uint64_t, double>), types::f64);
            case 0xB2:
                POPAT((convert_s<float, int32_t>), types::i32);
            case 0xB3:
                POPAT((convert_u<float, uint32_t>), types::i32);
            case 0xB4:
                POPAT((convert_s<float, int64_t>), types::i64);
            case 0xB5:
                POPAT((convert_u<float, uint64_t>), types::i64);
            case 0xB6:
                POPAT((demote<float, double>), types::f64);
            case 0xB7:
                POPAT((convert_s<double, int32_t>), types::i32);
            case 0xB8:
                POPAT((convert_u<double, uint32_t>), types::i32);
            case 0xB9:
                POPAT((convert_s<double, int64_t>), types::i64);
            case 0xBA:
                POPAT((convert_u<double, uint64_t>), types::i64);
            case 0xBB:
                POPAT((promote<double, float>), types::f32);
            case 0xBC:
                POPAT((reinterpret<uint32_t, float>), types::f32);
            case 0xBD:
                POPAT((reinterpret<uint64_t, double>), types::f64);
            case 0xBE:
                POPAT((reinterpret<float, uint32_t>), types::i32);
            case 0xBF:
                POPAT((reinterpret<double, uint64_t>), types::i64);

            default:
                throw std::runtime_error("bad instruction " + std::to_string(which));
        }
    }

    #ifdef DEBUGGING
    lg::log("Left Expr");
    #endif // DEBUGGING
}

#if 0
info_stack::info_stack(context& _ctx, runtime::store& s, full_stack& _full, const runtime::funcaddr& address, runtime::moduleinst& minst) : ctx(_ctx), full(_full), in(start_function(s, full, address, minst))
{
    type = 1;
}

info_stack::info_stack(context& _ctx, const label& l, const types::vec<types::instr>& exp, full_stack& _full) : ctx(_ctx), full(_full), in(start_label(ctx, l, exp, full))
{
    type = 2;
}

const types::vec<types::instr>& info_stack::start_function(runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    const runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    int num_args = ftype.params.size();

    if(std::holds_alternative<runtime::webasm_func>(finst.funct))
    {
        const runtime::webasm_func& fnc = std::get<runtime::webasm_func>(finst.funct);

        types::vec<runtime::value> popped = full.pop_num_vals(num_args);

        /*std::cout <<" NUM ARGS " << num_args << std::endl;

        for(auto& i : popped)
        {
            std::cout << i.friendly_val() << std::endl;
        }*/

        types::vec<types::local> local_types = fnc.funct.fnc.locals;
        const types::expr& expression = fnc.funct.fnc.e;

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
        fr.inst = &minst;
        fr.locals = popped;
        fr.locals = fr.locals.append(local_zeroes);

        activation activate;
        activate.return_arity = types::s32{ftype.results.size()};
        activate.f = fr;

        full.push_activation(activate);

        return expression.i;
    }
    else
    {
        throw std::runtime_error("unimplemented;");
    }
}

const types::vec<types::instr>& info_stack::start_label(context& ctx, const label& l, const types::vec<types::instr>& exp, full_stack& full)
{
    should_loop = false;

    full.push_label(l);

    if(has_delayed_values_push)
    {
        if(ctx.capture_arity)
            full.push_values(ctx.capture_val);

        ctx.capture_arity = 0;

        has_delayed_values_push = false;
    }

    return exp;
}

void info_stack::end_label(context& ctx, full_stack& full)
{
    if(ctx.frame_abort)
    {
        full.pop_all_values_on_stack_unsafe_nocatch();
        full.pop_back_label();

        should_loop = false;
        return;
    }

    ///we SHOULD do pop all values on stack, but instead we cheat with backup vals
    ///to exploit the fact that they're not shared stacks
    if(ctx.abort_stack > 0)
    {
        full.pop_all_values_on_stack_unsafe_nocatch();
        full.pop_back_label();

        ctx.abort_stack--;

        if(ctx.abort_stack == 0)
        {
            if(ctx.continuation != 2)
            {
                if(ctx.capture_arity)
                    full.push_values(ctx.capture_val);

                ctx.capture_arity = 0;
            }
            else
            {
                has_delayed_values_push = true;
            }

            if(ctx.continuation == 2)
            {
                ///loop and start again from beginning
                should_loop = true;
            }

            if(ctx.continuation == 0)
            {
                throw std::runtime_error("Bad continuation, 0");
            }
        }
    }
    else
    {
        //int backup_vals = full.current_stack_size();

        //full.pop_back_label();

        full.label_stack.pop_back();

        int last = full.stack_start_sizes.back();
        full.stack_start_sizes.pop_back();

        full.stack_start_sizes.back() += last;

        //full.stack_start_sizes.push_back(backup_vals);
    }
}

void info_stack::destroy()
{
    if(type == 1)
    {
        end_function(ctx, full);
    }

    if(type == 2)
    {
        end_label(ctx, full);
    }
}

bool info_stack::loop()
{
    return should_loop;
}

void info_stack::end_function(context& ctx, full_stack& full)
{
    if(!ctx.frame_abort)
    {
        activation& current = full.get_current();

        types::vec<runtime::value> found = full.pop_num_vals((int32_t)current.return_arity);

        full.pop_back_activation();

        full.push_all_values(found);
    }
    else if(ctx.frame_abort)
    {
        ctx.frame_abort = false;

        full.pop_all_values_on_stack_unsafe_nocatch();
        full.pop_back_activation();

        if(ctx.capture_arity)
            full.push_values(ctx.capture_val);

        ctx.capture_arity = 0;
    }
}

types::vec<runtime::value> info_stack::end_function_final(context& ctx, full_stack& full)
{
    if(!ctx.frame_abort)
    {
        activation& current = full.get_current();

        types::vec<runtime::value> found = full.pop_num_vals((int32_t)current.return_arity);

        full.pop_back_activation();

        full.push_all_values(found);

        return found;
    }
    else if(ctx.frame_abort)
    {
        ctx.frame_abort = false;

        full.pop_all_values_on_stack_unsafe_nocatch();
        full.pop_back_activation();

        auto bval = ctx.capture_val;

        /*full.push_all_values(ctx.capture_vals);

        ctx.capture_vals.clear();*/

        if(ctx.capture_arity)
            full.push_values(ctx.capture_val);

        ctx.capture_arity = 0;

        return {bval};
    }

    throw std::runtime_error("unreachable");
}

void do_push_label(context& ctx, label& to_push, const types::vec<types::instr>& in, full_stack& full, types::vec<info_stack>& istack, int cpc, info_stack*& current_stack)
{
    current_stack->pc = cpc + 1;

    istack.emplace_back(ctx, to_push, in, full);

    current_stack = &istack.back();
    current_stack->should_loop = true;
}

void do_push_activation(context& ctx, runtime::store& s, runtime::moduleinst& minst, runtime::funcaddr& faddr_to_push, full_stack& full, types::vec<info_stack>& istack, int cpc, info_stack*& current_stack)
{
    current_stack->pc = cpc + 1;

    istack.emplace_back(ctx, s, full, faddr_to_push, minst);

    current_stack = &istack.back();
    current_stack->should_loop = true;
}

types::vec<runtime::value> entry_func(context& ctx, runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst)
{
    types::vec<info_stack>& istack = ctx.istack;

    istack.emplace_back(ctx, s, full, address, minst);

    types::vec<runtime::value> return_vals;

    while(istack.size() > 0)
    {
        info_stack* current_stack = &istack[istack.size() - 1];
        current_stack->should_loop = true;

        ///so
        ///we need to loop through the instructions in the current info stack
        ///then we need to execute the instructions
        ///at the other end in evaluating an op, if we hit a condition we should add to the info stack, and then act depending on the result condition

        #ifdef DEBUGGING
        //std::cout <<"istack\n";
        //std::cout << "frame abort? " << ctx.frame_abort << " abort stack " << ctx.abort_stack << std::endl;
        #endif // DEBUGGING

        //types::instr::assert_on_destruct = true;

        while(current_stack->loop() && !ctx.frame_abort && ctx.abort_stack == 0)
        {
            current_stack->should_loop = false;

            ///this is... always 0?
            int ilen = current_stack->pc;
            const types::vec<types::instr>& in = current_stack->in;

            int len = in.size();

            for(; ilen < len; ilen++)
            {
                const types::instr& is = in[ilen];

                uint8_t which = is.which;

                #ifdef PERF
                prof.add(which);
                #endif // PERF


                #ifdef DEBUGGING
                std::cout << "0x" << std::hex << (uint32_t)which << std::dec << " " << ctx.expression_counter << std::endl;
                #endif // DEBUGGING

                /*lg::log("0x");
                lg::log_hex_noline(which);
                lg::log(", ");*/

                ///good lord this is tedious
                switch(which)
                {
                    case 0x00:
                        throw std::runtime_error("unreachable");
                        break;

                    case 0x01:
                        break;

                    case 0x02:
                    {
                        const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                        label l;
                        l.btype = sbd.btype;
                        l.continuation = 1;

                        do_push_label(ctx, l, sbd.first, full, istack, ilen, current_stack);

                        ilen = len;

                        break;
                    }

                    case 0x03:
                    {
                        const types::single_branch_data& sbd = std::get<types::single_branch_data>(is.dat);

                        label l;
                        l.btype = sbd.btype;
                        l.continuation = 2;

                        if(l.btype.arity() != 0)
                            throw std::runtime_error("Wrong arity?");

                        do_push_label(ctx, l, sbd.first, full, istack, ilen, current_stack);

                        ilen = len;

                        break;
                    }

                    case 0x04:
                    {
                        const types::double_branch_data& dbd = std::get<types::double_branch_data>(is.dat);

                        label l;
                        l.btype = dbd.btype;
                        l.continuation = 3;

                        runtime::value val = full.pop_back();

                        if(!val.is_i32())
                            throw std::runtime_error("0x04 if/else expected i32");

                        types::i32 type = std::get<types::i32>(val.v);

                        uint32_t c = type.val;

                        if(c != 0)
                        {
                            do_push_label(ctx, l, dbd.first, full, istack, ilen, current_stack);
                        }
                        else
                        {
                            do_push_label(ctx, l, dbd.second, full, istack, ilen, current_stack);
                        }

                        ilen = len;

                        break;
                    }

                    case 0x0C:
                    {
                        types::labelidx lidx = std::get<types::labelidx>(is.dat);

                        uint32_t idx = (uint32_t)lidx;

                        if((uint32_t)full.num_labels() < idx + 1)
                        {
                            throw std::runtime_error("not enough labels");
                        }

                        fjump(ctx, lidx, full);

                        current_stack->pc = ilen + 1;
                        ilen = len;

                        //lg::log("hit br ", std::to_string(idx));

                        break;
                    }

                    case 0x0D:
                    {
                        runtime::value val = full.pop_back();

                        if(!val.is_i32())
                            throw std::runtime_error("expected i32 in 0x0D");

                        types::i32 type = std::get<types::i32>(val.v);

                        #ifdef DEBUGGING
                        lg::log("hit br_if");
                        #endif // DEBUGGING

                        if((uint32_t)type != 0)
                        {
                            types::labelidx lidx = std::get<types::labelidx>(is.dat);

                            fjump(ctx, lidx, full);

                            uint32_t idx = (uint32_t)lidx;

                            if((uint32_t)full.num_labels() < idx + 1)
                            {
                                throw std::runtime_error("not enough labels");
                            }

                            current_stack->pc = ilen + 1;
                            ilen = len;

                            #ifdef DEBUGGING
                            lg::log("took branch to ", std::to_string(idx));
                            #endif // DEBUGGING
                        }

                        break;
                    }

                    case 0x0E:
                    {
                        types::br_table_data br_td = std::get<types::br_table_data>(is.dat);

                        runtime::value top_val = full.pop_back();

                        if(!top_val.is_i32())
                            throw std::runtime_error("br_td should have been i32");

                        types::i32 val = std::get<types::i32>(top_val.v);

                        uint32_t idx = (uint32_t)val;

                        if(idx < (uint32_t)br_td.labels.size())
                        {
                            types::labelidx lidx = br_td.labels[idx];

                            fjump(ctx, lidx, full);
                        }
                        else
                        {
                            fjump(ctx, br_td.fin, full);
                        }

                        current_stack->pc = ilen + 1;
                        ilen = len;

                        break;
                    }

                    case 0x0F:
                    {
                        #ifdef DEBUGGING
                        lg::log("Stk size ", full.value_stack_size());
                        lg::log("THIS IS RETURNING HELLO ", full.peek_back().value_or(runtime::value{types::i32{543}}).friendly_val());
                        #endif // DEBUGGING

                        fjump_up_frame(ctx, full);

                        current_stack->pc = ilen + 1;
                        ilen = len;

                        break;
                    }

                    case 0x10:
                    {
                        types::funcidx fidx = std::get<types::funcidx>(is.dat);

                        uint32_t idx = (uint32_t)fidx;
                        activation& activate = full.get_current();

                        if(idx >= (uint32_t)activate.f.inst->funcaddrs.size())
                            throw std::runtime_error("Bad fidx in 0x10");

                        #ifdef DEBUGGING
                        lg::log("calling hi there, value stack size is ", full.value_stack_size());
                        #endif // DEBUGGING

                        do_push_activation(ctx, s, minst, activate.f.inst->funcaddrs[idx], full, istack, ilen, current_stack);

                        ilen = len;

                        break;
                    }

                    case 0x11:
                    {
                        ///alright indirect calls
                        types::funcidx found_fidx = std::get<types::funcidx>(is.dat);

                        activation& activate = full.get_current();

                        runtime::moduleinst* inst = activate.f.inst;

                        if(inst->tableaddrs.size() < 1)
                            throw std::runtime_error("Failed table addr < 1 in indirect call");

                        runtime::tableaddr taddr = inst->tableaddrs[0];

                        uint32_t tidx = (uint32_t)taddr;

                        if(tidx >= (uint32_t)s.tables.size())
                            throw std::runtime_error("Bad tidx in indirect call");

                        runtime::tableinst& tinst = s.tables[tidx];

                        uint32_t fidx = (uint32_t)found_fidx;

                        if(fidx >= (uint32_t)inst->typel.size())
                            throw std::runtime_error("Bad fidx");

                        types::functype ft_expect = inst->typel[fidx];


                        runtime::value val = full.pop_back();

                        if(!val.is_i32())
                            throw std::runtime_error("Not i32 in call indirect");

                        types::i32 i_call = std::get<types::i32>(val.v);

                        uint32_t i_call_idx = (uint32_t)i_call;

                        if(i_call_idx >= (uint32_t)tinst.elem.size())
                            throw std::runtime_error("Bad i_call in indirect call");

                        if(!tinst.elem[i_call_idx].addr.has_value())
                            throw std::runtime_error("No faddr in indirect call");

                        runtime::funcaddr runtime_addr = tinst.elem[i_call_idx].addr.value();

                        uint32_t runtime_idx = (uint32_t)runtime_addr;

                        if(runtime_idx >= (uint32_t)s.funcs.size())
                            throw std::runtime_error("Runtime idx oob (validation)");

                        runtime::funcinst& finst = s.funcs[runtime_idx];

                        types::functype ft_actual = finst.type;

                        if(!types::funcs_equal(ft_actual, ft_expect))
                            throw std::runtime_error("Expected and actual types of funcs differ");

                        do_push_activation(ctx, s, minst, runtime_addr, full, istack, ilen, current_stack);

                        ilen = len;

                        break;
                    }

                    case 0x1A:
                    {
                        full.pop_back();

                        break;
                    }

                    case 0x1B:
                    {
                        select(full);

                        break;
                    }

                    case 0x20:
                        INVOKE_LOCAL(get_local);
                    case 0x21:
                        INVOKE_LOCAL(set_local);
                    case 0x22:
                        INVOKE_LOCAL(tee_local);
                    case 0x23:
                        INVOKE_GLOBAL(get_global);
                    case 0x24:
                        INVOKE_GLOBAL(set_global);

                    case 0x28:
                        MEM_LOAD(uint32_t, uint32_t);
                    case 0x29:
                        MEM_LOAD(uint64_t, uint64_t);
                    case 0x2A:
                        MEM_LOAD(float, float);
                    case 0x2B:
                        MEM_LOAD(double, double);
                    case 0x2C:
                        MEM_LOAD(int32_t, int8_t);
                    case 0x2D:
                        MEM_LOAD(uint32_t, uint8_t);
                    case 0x2E:
                        MEM_LOAD(int32_t, int16_t);
                    case 0x2F:
                        MEM_LOAD(uint32_t, uint16_t);
                    case 0x30:
                        MEM_LOAD(int64_t, int8_t);
                    case 0x31:
                        MEM_LOAD(uint64_t, uint8_t);
                    case 0x32:
                        MEM_LOAD(int64_t, int16_t);
                    case 0x33:
                        MEM_LOAD(uint64_t, uint16_t);
                    case 0x34:
                        MEM_LOAD(int64_t, int32_t);
                    case 0x35:
                        MEM_LOAD(uint64_t, uint32_t);

                    case 0x36:
                        MEM_STORE(uint32_t, 4);
                    case 0x37:
                        MEM_STORE(uint64_t, 4);
                    case 0x38:
                        MEM_STORE(float, 4);
                    case 0x39:
                        MEM_STORE(double, 4);
                    case 0x3A:
                        MEM_STORE(uint32_t, 1);
                    case 0x3B:
                        MEM_STORE(uint32_t, 2);
                    case 0x3C:
                        MEM_STORE(uint64_t, 1);
                    case 0x3D:
                        MEM_STORE(uint64_t, 2);
                    case 0x3E:
                        MEM_STORE(uint64_t, 4);

                    case 0x3F:
                        MEM_SIZE();
                    case 0x40:
                        MEM_GROW();

                    ///TODO size and grow

                    case 0x41:
                        PUSH_CONSTANT(types::i32);
                    case 0x42:
                        PUSH_CONSTANT(types::i64);
                    case 0x43:
                        PUSH_CONSTANT(types::f32);
                    case 0x44:
                        PUSH_CONSTANT(types::f64);

                    case 0x45:
                        POPAT(ieqz<uint32_t>, types::i32);
                    case 0x46:
                        POPBT(eq<uint32_t>, types::i32);
                    case 0x47:
                        POPBT(ne<uint32_t>, types::i32);
                    case 0x48:
                        POPBT(ilt_s<int32_t>, types::i32);
                    case 0x49:
                        POPBT(ilt_u<uint32_t>, types::i32);
                    case 0x4A:
                        POPBT(igt_s<int32_t>, types::i32);
                    case 0x4B:
                        POPBT(igt_u<uint32_t>, types::i32);
                    case 0x4C:
                        POPBT(ile_s<int32_t>, types::i32);
                    case 0x4D:
                        POPBT(ile_u<uint32_t>, types::i32);
                    case 0x4E:
                        POPBT(ige_s<int32_t>, types::i32);
                    case 0x4F:
                        POPBT(ige_u<uint32_t>, types::i32);

                    case 0x50:
                        POPAT(ieqz<uint64_t>, types::i64);
                    case 0x51:
                        POPBT(eq<uint64_t>, types::i64);
                    case 0x52:
                        POPBT(ne<uint64_t>, types::i64);
                    case 0x53:
                        POPBT(ilt_s<int64_t>, types::i64);
                    case 0x54:
                        POPBT(ilt_u<uint64_t>, types::i64);
                    case 0x55:
                        POPBT(igt_s<int64_t>, types::i64);
                    case 0x56:
                        POPBT(igt_u<uint64_t>, types::i64);
                    case 0x57:
                        POPBT(ile_s<int64_t>, types::i64);
                    case 0x58:
                        POPBT(ile_u<uint64_t>, types::i64);
                    case 0x59:
                        POPBT(ige_s<int64_t>, types::i64);
                    case 0x5A:
                        POPBT(ige_u<uint64_t>, types::i64);

                    case 0x5B:
                        POPBT(eq<float>, types::f32);
                    case 0x5C:
                        POPBT(ne<float>, types::f32);
                    case 0x5D:
                        POPBT(flt<float>, types::f32);
                    case 0x5E:
                        POPBT(fgt<float>, types::f32);
                    case 0x5F:
                        POPBT(fle<float>, types::f32);
                    case 0x60:
                        POPBT(fge<float>, types::f32);

                    case 0x61:
                        POPBT(eq<double>, types::f64);
                    case 0x62:
                        POPBT(ne<double>, types::f64);
                    case 0x63:
                        POPBT(flt<double>, types::f64);
                    case 0x64:
                        POPBT(fgt<double>, types::f64);
                    case 0x65:
                        POPBT(fle<double>, types::f64);
                    case 0x66:
                        POPBT(fge<double>, types::f64);

                    case 0x67:
                        POPAT(iclz<uint32_t>, types::i32);
                    case 0x68:
                        POPAT(ictz<uint32_t>, types::i32);
                    case 0x69:
                        POPAT(ipopcnt<uint32_t>, types::i32);
                    case 0x6A:
                        POPBT(add<uint32_t>, types::i32);
                    case 0x6B:
                        POPBT(sub<uint32_t>, types::i32);
                    case 0x6C:
                        POPBT(mul<uint32_t>, types::i32);
                    case 0x6D:
                        POPBT(idiv<int32_t>, types::i32);
                    case 0x6E:
                        POPBT(idiv<uint32_t>, types::i32);
                    case 0x6F:
                        POPBT(remi<int32_t>, types::i32);
                    case 0x70:
                        POPBT(remi<uint32_t>, types::i32);
                    case 0x71:
                        POPBT(iand<uint32_t>, types::i32);
                    case 0x72:
                        POPBT(ior<uint32_t>, types::i32);
                    case 0x73:
                        POPBT(ixor<uint32_t>, types::i32);
                    case 0x74:
                        POPBT(ishl<uint32_t>, types::i32);
                    case 0x75:
                        POPBT(ishr_s<int32_t>, types::i32);
                    case 0x76:
                        POPBT(ishr_u<uint32_t>, types::i32);
                    case 0x77:
                        POPBT(irotl<uint32_t>, types::i32);
                    case 0x78:
                        POPBT(irotr<uint32_t>, types::i32);


                    case 0x79:
                        POPAT(iclz<uint64_t>, types::i64);
                    case 0x7A:
                        POPAT(ictz<uint64_t>, types::i64);
                    case 0x7B:
                        POPAT(ipopcnt<uint64_t>, types::i64);
                    case 0x7C:
                        POPBT(add<uint64_t>, types::i64);
                    case 0x7D:
                        POPBT(sub<uint64_t>, types::i64);
                    case 0x7E:
                        POPBT(mul<uint64_t>, types::i64);
                    case 0x7F:
                        POPBT(idiv<int64_t>, types::i64);
                    case 0x80:
                        POPBT(idiv<uint64_t>, types::i64);
                    case 0x81:
                        POPBT(remi<int64_t>, types::i64);
                    case 0x82:
                        POPBT(remi<uint64_t>, types::i64);
                    case 0x83:
                        POPBT(iand<uint64_t>, types::i64);
                    case 0x84:
                        POPBT(ior<uint64_t>, types::i64);
                    case 0x85:
                        POPBT(ixor<uint64_t>, types::i64);
                    case 0x86:
                        POPBT(ishl<uint64_t>, types::i64);
                    case 0x87:
                        POPBT(ishr_s<int64_t>, types::i64);
                    case 0x88:
                        POPBT(ishr_u<uint64_t>, types::i64);
                    case 0x89:
                        POPBT(irotl<uint64_t>, types::i64);
                    case 0x8A:
                        POPBT(irotr<uint64_t>, types::i64);

                    case 0x8B:
                        POPAT(absf<float>, types::f32);
                    case 0x8C:
                        POPAT(fneg<float>, types::f32);
                    case 0x8D:
                        POPAT(fceil<float>, types::f32);
                    case 0x8E:
                        POPAT(ffloor<float>, types::f32);
                    case 0x8F:
                        POPAT(ftrunc<float>, types::f32);
                    case 0x90:
                        POPAT(fnearest<float>, types::f32);
                    case 0x91:
                        POPAT(fsqrt<float>, types::f32);
                    case 0x92:
                        POPBT(add<float>, types::f32);
                    case 0x93:
                        POPBT(sub<float>, types::f32);
                    case 0x94:
                        POPBT(mul<float>, types::f32);
                    case 0x95:
                        POPBT(fdiv<float>, types::f32);
                    case 0x96:
                        POPBT(fmin<float>, types::f32);
                    case 0x97:
                        POPBT(fmax<float>, types::f32);
                    case 0x98:
                        POPBT(fcopysign<float>, types::f32);

                    case 0x99:
                        POPAT(absf<double>, types::f64);
                    case 0x9A:
                        POPAT(fneg<double>, types::f64);
                    case 0x9B:
                        POPAT(fceil<double>, types::f64);
                    case 0x9C:
                        POPAT(ffloor<double>, types::f64);
                    case 0x9D:
                        POPAT(ftrunc<double>, types::f64);
                    case 0x9E:
                        POPAT(fnearest<double>, types::f64);
                    case 0x9F:
                        POPAT(fsqrt<double>, types::f64);
                    case 0xA0:
                        POPBT(add<double>, types::f64);
                    case 0xA1:
                        POPBT(sub<double>, types::f64);
                    case 0xA2:
                        POPBT(mul<double>, types::f64);
                    case 0xA3:
                        POPBT(fdiv<double>, types::f64);
                    case 0xA4:
                        POPBT(fmin<double>, types::f64);
                    case 0xA5:
                        POPBT(fmax<double>, types::f64);
                    case 0xA6:
                        POPBT(fcopysign<double>, types::f64);

                    ///these functions are all template parameter format
                    ///<dest, src>
                    ///so trunc_s takes the argument as a float
                    ///and then converts it to an in32_t
                    case 0xA7:
                        POPAT((wrap<uint32_t, 64>), types::i32);
                    case 0xA8:
                        POPAT((trunc_s<int32_t, float>), types::f32);
                    case 0xA9:
                        POPAT((trunc_u<uint32_t, float>), types::f32);
                    case 0xAA:
                        POPAT((trunc_s<int32_t, double>), types::f64);
                    case 0xAB:
                        POPAT((trunc_u<uint32_t, double>), types::f64);
                    case 0xAC:
                        POPAT((extend_s<int64_t, int32_t>), types::i32);
                    case 0xAD:
                        POPAT((extend_u<uint64_t, int32_t>), types::i32);
                    case 0xAE:
                        POPAT((trunc_s<int64_t, float>), types::f32);
                    case 0xAF:
                        POPAT((trunc_u<uint64_t, float>), types::f32);
                    case 0xB0:
                        POPAT((trunc_s<int64_t, double>), types::f64);
                    case 0xB1:
                        POPAT((trunc_u<uint64_t, double>), types::f64);
                    case 0xB2:
                        POPAT((convert_s<float, int32_t>), types::i32);
                    case 0xB3:
                        POPAT((convert_u<float, uint32_t>), types::i32);
                    case 0xB4:
                        POPAT((convert_s<float, int64_t>), types::i64);
                    case 0xB5:
                        POPAT((convert_u<float, uint64_t>), types::i64);
                    case 0xB6:
                        POPAT((demote<float, double>), types::f64);
                    case 0xB7:
                        POPAT((convert_s<double, int32_t>), types::i32);
                    case 0xB8:
                        POPAT((convert_u<double, uint32_t>), types::i32);
                    case 0xB9:
                        POPAT((convert_s<double, int64_t>), types::i64);
                    case 0xBA:
                        POPAT((convert_u<double, uint64_t>), types::i64);
                    case 0xBB:
                        POPAT((promote<double, float>), types::f32);
                    case 0xBC:
                        POPAT((reinterpret<uint32_t, float>), types::f32);
                    case 0xBD:
                        POPAT((reinterpret<uint64_t, double>), types::f64);
                    case 0xBE:
                        POPAT((reinterpret<float, uint32_t>), types::i32);
                    case 0xBF:
                        POPAT((reinterpret<double, uint64_t>), types::i64);

                    default:
                        throw std::runtime_error("bad instruction " + std::to_string(which));
                }
            }
        }

        /*if(istack.size() == 1)
        {
            return_vals = full.pop_all_values_on_stack_unsafe();
        }*/

        ///? I think this is impossible to hit
        //istack.back().should_loop = false;

        if(istack.back().type == 2)
        {
            if(ctx.abort_stack == 1 && ctx.continuation == 2)
            {
                label clab = full.get_current_label();

                #ifdef DEBUGGING
                std::cout << "type " << istack.back().type << std::endl;
                #endif

                istack.back().destroy();

                if(istack.back().should_loop)
                {
                    info_stack& lst = istack.back();

                    istack.back().start_label(ctx, clab, lst.in, full);
                    istack.back().pc = 0;

                    //std::cout << "hello\n";

                    continue;
                }
            }
            else
            {
                istack.back().destroy();
            }
        }
        else if(istack.back().type == 1)
        {
            if(istack.size() > 1)
                istack.back().end_function(ctx, full);
            else
                return_vals = istack.back().end_function_final(ctx, full);
        }

        /*if(istack.size() == 1)
        {
            return_vals = full.pop_all_values_on_stack_unsafe();
        }*/

        istack.pop_back();
    }

    return return_vals;
}
#endif // 0

types::vec<runtime::value> eval_with_frame(runtime::moduleinst& minst, runtime::store& s, const types::vec<types::instr>& exp)
{
    types::vec<info_stack> istack;
    full_stack full;
    context ctx(istack);

    frame fr;
    ///SUPER BAD CODE ALERT
    fr.inst = &minst;

    activation activate;
    activate.return_arity = {0};
    activate.f = fr;

    full.push_activation(activate);

    ctx.current_arity = 0;

    eval_expr(ctx, s, exp, full, activate);

    if(!ctx.frame_abort)
    {
        types::vec<runtime::value> found = full.pop_all_values_on_stack_unsafe();

        full.pop_back_activation();

        full.push_all_values(found);

        return found;
    }
    else if(ctx.frame_abort)
    {
        ctx.frame_abort = false;

        full.pop_all_values_on_stack_unsafe_nocatch();
        full.pop_back_activation();

        auto bval = ctx.capture_val;

        if(ctx.capture_arity)
            full.push_values(ctx.capture_val);

        ctx.capture_arity = 0;

        return {bval};
    }

    throw std::runtime_error("unreachable");
}

void eval_with_label(context& ctx, runtime::store& s, const label& l, const types::vec<types::instr>& exp, full_stack& full, activation& activate)
{
    bool should_loop = true;
    bool has_delayed_values_push = false;

    ///ok
    ///so basically I really want to avoid this push here in this loop
    ///as the loop gets called rather a lot
    full.push_label(l);

    while(should_loop)
    {
        should_loop = false;

        if(has_delayed_values_push)
        {
            if(ctx.capture_arity)
                full.push_values(ctx.capture_val);

            ctx.capture_arity = 0;

            has_delayed_values_push = false;
        }

        ctx.current_arity = l.btype.arity();

        eval_expr(ctx, s, exp, full, activate);

        auto all_vals = full.pop_all_values_on_stack_unsafe();

        //full.ensure_label();

        if(ctx.frame_abort)
        {
            full.pop_back_label();
            return;
        }

        if(ctx.abort_stack > 0)
        {
            ctx.abort_stack--;

            if(ctx.abort_stack == 0)
            {
                if(l.continuation == 2)
                {
                    has_delayed_values_push = true;
                    ///loop and start again from beginning
                    should_loop = true;

                    ///this is equivalent to popping the values off the stack, then them being pushed next time round the loop again
                    full.stack_start_sizes.back() = 0;

                    #ifdef DEBUGGING
                    lg::log("continuation pt 2 loop\n");
                    #endif // DEBUGGING
                }
                else
                {
                    full.pop_back_label();

                    if(ctx.capture_arity)
                        full.push_values(ctx.capture_val);

                    ctx.capture_arity = 0;

                    return;
                }

                if(l.continuation == 0)
                {
                    throw std::runtime_error("Bad continuation, 0");
                }
            }
        }
        else
        {
            full.pop_back_label();

            full.push_all_values(all_vals);

            return;
        }
    }

    full.pop_back_label();

    //throw std::runtime_error("Unreachable");

    ///ok so
    ///we only want to pop the label if we're guaranteed to terminate the loop right?
    //full.pop_back_label();
}

///alright: transforming from recursive to linear plan
///so currently there's invoke_intl, which calls eval_expr
///and there's eval_with_label which calls eval_expr
///eval_expr calls both invoke_intl and eval_expr
///so maybe initially unify invoke_intl and eval_with_label with a flat (note that the return value of invoke_intl is ignored everywhere internally)
types::vec<runtime::value> invoke_intl(context& ctx, runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)s.funcs.size())
        throw std::runtime_error("Adr out of bounds");

    //types::instr::assert_on_destruct++;

    runtime::funcinst& finst = s.funcs[adr];

    types::functype ftype = finst.type;

    /*if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch");*/

    int num_args = ftype.params.size();

    if(std::holds_alternative<runtime::webasm_func>(finst.funct))
    {
        const runtime::webasm_func& fnc = std::get<runtime::webasm_func>(finst.funct);

        types::vec<runtime::value> popped = full.pop_num_vals(num_args);

        types::vec<types::local> local_types = fnc.funct.fnc.locals;
        const types::expr& expression = fnc.funct.fnc.e;

        #ifdef DEBUGGING
        lg::log("Function has ", num_args, " arguments and returns ", ftype.results.size(), " values");
        #endif // DEBUGGING

        types::vec<runtime::value> local_zeroes;

        for(const types::local& loc : local_types)
        {
            for(uint32_t i=0; i < (uint32_t)loc.n; i++)
            {
                runtime::value val;
                val.from_valtype(loc.type);

                //lg::log("asdflocal ", val.friendly_val());

                local_zeroes.push_back(val);
            }
        }

        frame fr;
        ///SUPER BAD CODE ALERT
        fr.inst = &minst;

        fr.locals = popped;
        fr.locals = fr.locals.append(local_zeroes);

        /*lg::log("fr locals ", fr.locals.size());
        lg::log("ltypes ", local_types.size());
        lg::log("hello ", local_zeroes.size());*/

        activation activate;
        activate.return_arity = types::s32{ftype.results.size()};
        activate.f = fr;


        #ifdef DEBUGGING
        lg::logn("Returns ", ftype.results.size(), " values, expects ", num_args, " args. Arg is ");

        for(auto& i : popped)
        {
            lg::logn(i.friendly_val(), " ");
        }

        lg::log("");
        #endif // DEBUGGING


        //lg::log("push");
        full.push_activation(activate);

        ctx.current_arity = 0;

        eval_expr(ctx, s, expression.i, full, activate);

        if(!ctx.frame_abort)
        {
            types::vec<runtime::value> found = full.pop_num_vals((int32_t)activate.return_arity);

            full.pop_back_activation();

            full.push_all_values(found);

            //types::instr::assert_on_destruct--;

            return found;
        }
        else if(ctx.frame_abort)
        {
            ctx.frame_abort = false;

            full.pop_all_values_on_stack_unsafe_nocatch();
            full.pop_back_activation();

            auto bval = ctx.capture_val;

            if(ctx.capture_arity)
                full.push_values(ctx.capture_val);

            ctx.capture_arity = 0;

            //types::instr::assert_on_destruct--;

            return {bval};
        }

        //lg::log("pop");


        ///carry on instruction stream after the call
        ///as you do
    }
    else
    {
        throw std::runtime_error("Bad function invocation type");
    }

    //types::instr::assert_on_destruct--;

    return types::vec<runtime::value>();
}

types::vec<runtime::value> runtime::store::invoke(const runtime::funcaddr& address, runtime::moduleinst& minst, const types::vec<runtime::value>& vals)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = funcs[adr];

    types::functype ftype = finst.type;

    if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch");

    full_stack full;
    types::vec<info_stack> istack;

    for(auto& val : vals)
    {
        full.push_values(val);
    }

    context ctx(istack);

    //types::vec<runtime::value> return_value = entry_func(ctx, *this, full, address, minst);

    types::vec<runtime::value> return_value = invoke_intl(ctx, *this, full, address, minst);

    #ifdef DEBUGGING
    lg::log("left on stack ", full.full.size());
    #endif // DEBUGGING

    return return_value;

    ///pop val from stack

    /*if(ftype.results.size() > 0)
    {
        auto res = full.pop_num_vals(1);
    }*/
}

types::vec<runtime::value> runtime::store::invoke_by_name(const std::string& imported, moduleinst& minst, const types::vec<value>& vals)
{
    for(runtime::exportinst& einst : minst.exports)
    {
        if(einst.name == imported)
        {
            return invoke(std::get<runtime::funcaddr>(einst.value.val), minst, vals);
        }
    }

    throw std::runtime_error("No such function");
}
