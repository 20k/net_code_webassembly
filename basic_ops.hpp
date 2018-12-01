#ifndef BASIC_OPS_HPP_INCLUDED
#define BASIC_OPS_HPP_INCLUDED

#include <algorithm>
#include <math.h>
#include <iostream>
#include "logging.hpp"

template<typename T>
inline
bool bool_op(const T& t)
{
    return t > 0;
}

template<typename T>
inline
T add(const T& v1, const T& v2)
{
    return v1 + v2;
}

template<typename T>
inline
T sub(const T& v1, const T& v2)
{
    return v1 - v2;
}

template<typename T>
inline
T mul(const T& v1, const T& v2)
{
    return v1 * v2;
}

template<typename T>
inline
T idiv(const T& v1, const T& v2)
{
    if(v2 == 0)
        throw std::runtime_error("v2 == 0");

    return v1 / v2;
}

template<typename T>
inline
T fdiv(const T& v1, const T& v2)
{
    return v1 / v2;
}

template<typename T>
inline
T remi(const T& v1, const T& v2)
{
    if(v2 == 0)
        throw std::runtime_error("v2 == 0 in rem");

    //lg::log("mod ", v1, " ", v2);

    return v1 % v2;
}

template<typename T>
inline
T remf(const T& v1, const T& v2)
{
    return fmod(v1, v2);
}

template<typename T>
inline
T iand(const T& v1, const T& v2)
{
    return v1 & v2;
}

template<typename T>
inline
T ior(const T& v1, const T& v2)
{
    return v1 | v2;
}

template<typename T>
inline
T ixor(const T& v1, const T& v2)
{
    return v1 ^ v2;
}

template<typename T>
inline
T ishl(const T& v1, const T& v2)
{
    return v1 << v2;
}

///NEED TO CONVERT TYPES HERE
template<typename T>
inline
T ishr_u(const T& v1, const T& v2)
{
    return v1 >> v2;
}

template<typename T>
inline
T ishr_s(const T& v1, const T& v2)
{
    return v1 >> v2;
}

template<typename T>
inline
T irotl(const T& v1, const T& v2)
{
    return (v1 << v2)|(v1 >> ((sizeof(T)*8) - v2));
}

template<typename T>
inline
T irotr(const T& v1, const T& v2)
{
    return (v1 >> v2)|(v1 << (sizeof(T)*8 - v2));
}

template<typename T>
inline
T iclz(const T& v1)
{
    return __builtin_clz(v1);
}

template<typename T>
inline
T ictz(const T& v1)
{
    return __builtin_ctz(v1);
}

template<typename T>
inline
T ipopcnt(const T& v1)
{
    return __builtin_popcount(v1);
}

template<typename T>
inline
uint32_t ieqz(const T& v1)
{
    return v1 == 0;
}

template<typename T>
inline
uint32_t eq(const T& v1, const T& v2)
{
    return v1 == v2;
}

template<typename T>
inline
uint32_t ne(const T& v1, const T& v2)
{
    return v1 != v2;
}

template<typename T>
inline
uint32_t ilt_u(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
inline
uint32_t ilt_s(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
inline
uint32_t igt_u(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
inline
uint32_t igt_s(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
inline
uint32_t ile_u(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
inline
uint32_t ile_s(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
inline
uint32_t ige_u(const T& v1, const T& v2)
{
    return v1 >= v2;
}

template<typename T>
inline
uint32_t ige_s(const T& v1, const T& v2)
{
    return v1 >= v2;
}

template<typename T>
inline
T fmin(const T& v1, const T& v2)
{
    return std::min(v1, v2);
}

template<typename T>
inline
T fmax(const T& v1, const T& v2)
{
    return std::max(v1, v2);
}

template<typename T>
inline
T fcopysign(const T& v1, const T& v2)
{
    if(v1 > 0 && v2 > 0)
        return v1;

    if(v1 < 0 && v2 < 0)
        return v2;

    if(v1 > 0 && v2 < 0)
        return -v1;

    if(v1 < 0 && v2 > 0)
        return -v1;

    return v1;
}

template<typename T>
inline
T absf(const T& v1)
{
    return fabs(v1);
}

template<typename T>
inline
T fneg(const T& t)
{
    return -t;
}

template<typename T>
inline
T fsqrt(const T& t)
{
    return sqrt(t);
}

template<typename T>
inline
T fceil(const T& t)
{
    return ceil(t);
}

template<typename T>
inline
T ffloor(const T& t)
{
    return floor(t);
}

template<typename T>
inline
T ftrunc(const T& t)
{
    if(t > 0 && t < 1)
        return 0;

    if(t < 0 && t > -1)
        return -0;

    return (T)(int)t;
}

template<typename T>
inline
T fnearest(const T& t)
{
    if(t > 0 && t < 0.5)
        return 0;

    if(t < 0 && t > -0.5)
        return -0;

    return round(t);
}

template<typename T>
inline
uint32_t flt(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
inline
uint32_t fgt(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
inline
uint32_t fle(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
inline
uint32_t fge(const T& v1, const T& v2)
{
    return v1 >= v2;
}

template<typename T, typename U>
inline
T extend_u(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T extend_s(const U& in)
{
    return (T)in;
}

template<typename T, int N>
inline
T wrap(const T& in)
{
    return in % (T)pow(2, N);
}

template<typename T, typename U>
inline
T trunc_u(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T trunc_s(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T promote(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T demote(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T convert_u(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T convert_s(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
inline
T reinterpret(const U& in)
{
    return *(T*)&in;
}

template<typename T>
inline
T get_const(const T& t)
{
    return t;
}

template<typename T, typename U>
inline
void mem_load(runtime::store& s, const types::memarg& arg, full_stack& full, activation& activate)
{
    if(s.mems.size() < 1)
        throw std::runtime_error("No such mem idx 0");

    runtime::moduleinst* inst = activate.f.inst;

    if(inst == nullptr)
        throw std::runtime_error("inst == null");

    runtime::memaddr addr = inst->memaddrs[0];

    uint32_t raw_addr = (uint32_t)addr;

    if(raw_addr >= (uint32_t)s.mems.size())
        throw std::runtime_error("raw_addr > s.mems.size()");

    runtime::meminst& minst = s.mems[raw_addr];

    runtime::value top_i32 = full.pop_back();

    if(!top_i32.is_i32())
        throw std::runtime_error("Not i32 for mem load");

    uint32_t ea = (uint32_t)arg.offset + (uint32_t)std::get<types::i32>(top_i32.v);

    if(ea + sizeof(U) > (uint32_t)minst.dat.size())
        throw std::runtime_error("Out of memory (OOB in mem_load)");

    U str{0};
    memcpy(&str, &minst.dat[ea], sizeof(U));

    T ret = (T)str;

    runtime::value rval;
    rval.set(ret);

    full.push_values(rval);

    #ifdef DEBUGGING
    lg::log("Loaded ", (uint32_t)ret, " from ", ea);
    #endif // DEBUGGING

    //return ret;
}

#pragma GCC diagnostic ignored "-Warray-bounds"
template<typename T, int bytes>
inline
void mem_store(runtime::store& s, const types::memarg& arg, full_stack& full, activation& activate)
{
    static_assert(bytes <= sizeof(T));

    if(s.mems.size() < 1)
        throw std::runtime_error("No such mem idx 0");

    runtime::moduleinst* inst = activate.f.inst;

    if(inst == nullptr)
        throw std::runtime_error("inst == null");

    runtime::memaddr addr = inst->memaddrs[0];

    uint32_t raw_addr = (uint32_t)addr;

    if(raw_addr >= (uint32_t)s.mems.size())
        throw std::runtime_error("raw_addr > s.mems.size()");

    runtime::meminst& minst = s.mems[raw_addr];

    ///lets just pretend i did validation here
    ///normally its great but here i only have the underlying type, eg uint32_t
    ///whereas the runtime type is phrased in terms of i32
    ///it is possible to do but a faff

    runtime::value val = full.pop_back();
    runtime::value top_i32 = full.pop_back();

    if(!top_i32.is_i32())
        throw std::runtime_error("Not i32 for mem load");

    uint32_t found_bytes = (uint32_t)std::get<types::i32>(top_i32.v);

    uint32_t ea = (uint32_t)arg.offset + found_bytes;

    //lg::log("moffset ", found_bytes);

    if(ea + bytes > (uint32_t)minst.dat.size())
        throw std::runtime_error("Tried to hit OOB in mem_store");

    ///I believe the semantics for wrap are just integer truncation

    //val.apply([](auto concrete){if(sizeof(T) != sizeof(concrete)){throw std::runtime_error("BAD SIZE");}});

    val.apply([&minst, ea]
              (auto concrete)
              {
                  //if(bytes > sizeof(concrete))
                  //    throw std::runtime_error("Bad size");

                  memcpy(&minst.dat[ea], (char*)&concrete, bytes);

                  #ifdef DEBUGGING
                  lg::log("Stored ", (uint32_t)concrete, " to ", (uint32_t)ea);
                  #endif // DEBUGGING
              });
}

#pragma GCC diagnostic warning "-Warray-bounds"

/*inline
void tmalloc(runtime::store& s, const types::memarg& arg, full_stack& full)
{

}*/

inline
void memory_size(full_stack& full, runtime::store& s, activation& activate)
{
    activate.f.inst->memaddrs.check(0);
    runtime::memaddr maddr = activate.f.inst->memaddrs[0];

    s.mems.check((uint32_t)maddr);

    runtime::meminst& minst = s.mems[(uint32_t)maddr];

    uint32_t sz = minst.dat.size() / runtime::page_size;

    types::i32 ret{sz};

    full.push_values(ret);
}

inline
void memory_grow(full_stack& full, runtime::store& s, activation& activate)
{
    activate.f.inst->memaddrs.check(0);
    runtime::memaddr maddr = activate.f.inst->memaddrs[0];

    s.mems.check((uint32_t)maddr);

    runtime::meminst& minst = s.mems[(uint32_t)maddr];

    uint32_t sz = minst.dat.size() / runtime::page_size;

    //full.push_values(ret);

    runtime::value found_n = full.pop_back();

    if(!found_n.is_i32())
        throw std::runtime_error("Not i32 in mem grow");

    ///mem grow

    uint32_t pages = (uint32_t)std::get<types::i32>(found_n.v);

    #ifdef DEBUGGING
    std::cout << "old size " << minst.dat.size() << std::endl;
    #endif // DEBUGGING

    uint32_t total_pages = sz + pages;

    #ifdef DEBUGGING
    lg::log("Growing by ", pages, " pages");
    #endif // DEBUGGING

    if(total_pages * runtime::page_size > runtime::sandbox_mem_cap)
    {
        full.push_values(types::i32{-1});
    }
    else
    {
        minst.dat.resize(total_pages * runtime::page_size);

        #ifdef DEBUGGING
        std::cout << "new size " << minst.dat.size() << std::endl;
        #endif // DEBUGGING

        full.push_values(types::i32{sz});
    }
}

inline
void get_local(full_stack& full, const types::localidx& lidx, activation& activate)
{
    uint32_t idx = (uint32_t)lidx;

    if(idx >= (uint32_t)activate.f.locals.size())
        throw std::runtime_error("idx > max locals get_local");

    runtime::value val = activate.f.locals[idx];

    #ifdef DEBUGGING
    lg::log("got local ", val.friendly_val(), " from ", idx);
    #endif // DEBUGGING

    full.push_values(val);
}

inline
void set_local(full_stack& full, const types::localidx& lidx, activation& activate)
{
    runtime::value top = full.pop_back();

    uint32_t idx = (uint32_t)lidx;

    if(idx >= (uint32_t)activate.f.locals.size())
        throw std::runtime_error("idx > max locals get_local");

    activate.f.locals[idx] = top;

    #ifdef DEBUGGING
    lg::log("Set local ", top.friendly_val(), " at ", idx);
    #endif // DEBUGGING
}

inline
void tee_local(full_stack& full, const types::localidx& lidx, activation& activate)
{
    runtime::value top = full.pop_back();

    //lg::log("activate locals ", activate.f.locals.size());

    uint32_t idx = (uint32_t)lidx;

    if(idx >= (uint32_t)activate.f.locals.size())
        throw std::runtime_error("idx > max locals get_local");

    activate.f.locals[idx] = top;

    //lg::log("set idx ", std::to_string(idx), " to ", top.friendly_val());

    full.push_values(top);

    #ifdef DEBUGGING
    lg::log("set/got local ", top.friendly_val(), " at ", idx);
    #endif // DEBUGGING
}

inline
void get_global(runtime::store& s, full_stack& full, const types::globalidx& gidx, activation& activate)
{
    runtime::moduleinst* minst = activate.f.inst;

    uint32_t idx = (uint32_t)gidx;

    if(idx >= (uint32_t)minst->globaladdrs.size())
        throw std::runtime_error("bad idx in get_global");

    runtime::globaladdr addr = minst->globaladdrs[idx];

    if((uint32_t)addr >= (uint32_t)s.globals.size())
        throw std::runtime_error("bad addr in get_global");

    runtime::globalinst& glob = s.globals[(uint32_t)addr];

    runtime::value val = glob.val;

    full.push_values(val);
}

inline
void set_global(runtime::store& s, full_stack& full, const types::globalidx& gidx, activation& activate)
{
    runtime::moduleinst* minst = activate.f.inst;

    uint32_t idx = (uint32_t)gidx;

    if(idx >= (uint32_t)minst->globaladdrs.size())
        throw std::runtime_error("bad idx in get_global");

    runtime::globaladdr addr = minst->globaladdrs[idx];

    if((uint32_t)addr >= (uint32_t)s.globals.size())
        throw std::runtime_error("bad addr in get_global");

    runtime::globalinst& glob = s.globals[(uint32_t)addr];

    runtime::value val = full.pop_back();

    glob.val = val;
}

inline
void select(full_stack& full)
{
    runtime::value c = full.pop_back();

    if(!c.is_i32())
        throw std::runtime_error("Not i32 in select (3rd arg)");

    runtime::value v2 = full.pop_back();

    ///if this  is 1, leave val1 on the stack
    if(std::get<types::i32>(c.v).val != 0)
        return;

    ///is 0, therefore push val2 onto the stack
    full.pop_back();
    full.push_values(v2);
}

#endif // BASIC_OPS_HPP_INCLUDED
