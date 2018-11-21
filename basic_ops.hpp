#ifndef BASIC_OPS_HPP_INCLUDED
#define BASIC_OPS_HPP_INCLUDED


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


#endif // BASIC_OPS_HPP_INCLUDED
