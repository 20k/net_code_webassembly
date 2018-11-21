#ifndef BASIC_OPS_HPP_INCLUDED
#define BASIC_OPS_HPP_INCLUDED

#include <algorithm>
#include <math.h>

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
T idiv(const T& v1, const T& v2)
{
    if(v2 == 0)
        throw std::runtime_error("v2 == 0");

    return v1 / v2;
}

template<typename T>
T fdiv(const T& v1, const T& v2)
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
T eq(const T& v1, const T& v2)
{
    return v1 == v2;
}

template<typename T>
T ne(const T& v1, const T& v2)
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

template<typename T>
T fmin(const T& v1, const T& v2)
{
    return std::min(v1, v2);
}

template<typename T>
T fmax(const T& v1, const T& v2)
{
    return std::max(v1, v2);
}

template<typename T>
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
T absf(const T& v1)
{
    return fabs(v1);
}

template<typename T>
T fneg(const T& t)
{
    return -t;
}

template<typename T>
T fsqrt(const T& t)
{
    return sqrt(t);
}

template<typename T>
T fceil(const T& t)
{
    return ceil(t);
}

template<typename T>
T ffloor(const T& t)
{
    return floor(t);
}

template<typename T>
T ftrunc(const T& t)
{
    if(t > 0 && t < 1)
        return 0;

    if(t < 0 && t > -1)
        return -0;

    return (T)(int)t;
}

template<typename T>
T fnearest(const T& t)
{
    if(t > 0 && t < 0.5)
        return 0;

    if(t < 0 && t > -0.5)
        return -0;

    return round(t);
}

template<typename T>
T flt(const T& v1, const T& v2)
{
    return v1 < v2;
}

template<typename T>
T fgt(const T& v1, const T& v2)
{
    return v1 > v2;
}

template<typename T>
T fle(const T& v1, const T& v2)
{
    return v1 <= v2;
}

template<typename T>
T fge(const T& v1, const T& v2)
{
    return v1 >= v2;
}

template<typename T, typename U>
T extend_u(const U& in)
{
    return (T)&in;
}

template<typename T, typename U>
T extend_s(const U& in)
{
    return (T)in;
}

template<typename T, int N>
T wrap(const T& in)
{
    return in % (T)pow(2, N);
}

template<typename T, typename U>
T trunc_u(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T trunc_s(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T promote(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T demote(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T convert_u(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T convert_s(const U& in)
{
    return (T)in;
}

template<typename T, typename U>
T reinterpret(const U& in)
{
    return *(T*)&in;
}

template<typename T>
T get_const(const T& t)
{
    return t;
}

template<typename T, int bytes>
void mem_load(runtime::store& s, const types::memarg& arg, full_stack& full)
{
    if(s.mems.size() < 1)
        throw std::runtime_error("No such mem idx 0");

    activation& activate = full.get_current();

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

    if(ea + bytes >= (uint32_t)minst.dat.size())
        throw std::runtime_error("Out of memory");

    std::array<uint8_t, bytes> arr;

    for(uint32_t cbyte = 0; cbyte < bytes; cbyte++)
    {
        uint32_t current_offset = cbyte + ea;

        arr[cbyte] = minst.dat[current_offset];
    }

    T ret;
    memcpy(&ret, &arr[0], bytes);

    runtime::value rval;
    rval.set(ret);

    full.push_values(rval);

    //return ret;
}

#endif // BASIC_OPS_HPP_INCLUDED
