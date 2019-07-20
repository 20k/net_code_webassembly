#ifndef C_BASIC_OPS_HPP_INCLUDED
#define C_BASIC_OPS_HPP_INCLUDED

template<typename T>
std::string coerce(const std::string& in)
{
    throw std::runtime_error("Bad");
}

template<>
std::string coerce<uint32_t>(const std::string& in)
{
    return "(i32)" + in;
}

template<>
std::string coerce<int32_t>(const std::string& in)
{
    return "(int32_t)" + in;
}
template<>
std::string coerce<uint64_t>(const std::string& in)
{
    return "(i64)" + in;
}

template<>
std::string coerce<int64_t>(const std::string& in)
{
    return "(int64_t)" + in;
}

template<>
std::string coerce<float>(const std::string& in)
{
    return "(f32)" + in;
}

template<>
std::string coerce<double>(const std::string& in)
{
    return "(f64)" + in;
}

template<typename T>
std::string get_coerced(int val)
{
    return coerce<T>(get_variable_name(val));
}

#define GET(x) get_coerced<T>(x)

template<typename T>
std::string c_ieqz(int v1)
{
    return GET(v1) + " == 0;\n";
}

template<typename T>
std::string c_eq(int v1, int v2)
{
    return GET(v1) + " == " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ne(int v1, int v2)
{
    return GET(v1) + " != " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ilt_u(int v1, int v2)
{
    return GET(v1) + " < " + GET(v2) + ";\n";
}


template<typename T>
std::string c_ilt_s(int v1, int v2)
{
    return GET(v1) + " < " + GET(v2) + ";\n";
}

template<typename T>
std::string c_igt_u(int v1, int v2)
{
    return GET(v1) + " > " + GET(v2) + ";\n";
}

template<typename T>
std::string c_igt_s(int v1, int v2)
{
    return GET(v1) + " > " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ile_u(int v1, int v2)
{
    return GET(v1)  + " <= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ile_s(int v1, int v2)
{
    return GET(v1)  + " <= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ige_u(int v1, int v2)
{
    return GET(v1) + " >= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_ige_s(int v1, int v2)
{
    return GET(v1) + " >= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_flt(int v1, int v2)
{
    return GET(v1) + " < " + GET(v2) + ";\n";
}

template<typename T>
std::string c_fgt(int v1, int v2)
{
    return GET(v1) + " > " + GET(v2) + ";\n";
}

template<typename T>
std::string c_fle(int v1, int v2)
{
    return GET(v1) + " <= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_fge(int v1, int v2)
{
    return GET(v1) + " >= " + GET(v2) + ";\n";
}

template<typename T>
std::string c_add(int v1, int v2)
{
    return GET(v1) + " + " + GET(v2) + ";\n";
}

template<typename T>
std::string c_sub(int v1, int v2)
{
    return GET(v1) + " - " + GET(v2) + ";\n";
}

#define DOP(x, y) \
template<typename T> \
std::string x(int v1, int v2) \
{\
    return GET(v1) + " " #y " " + GET(v2) + ";\n"; \
}

#define DEXT(x, y) \
template<typename T> \
std::string x(int v1, int v2) \
{\
    return #y "(" + GET(v1) + ", " + GET(v2) + ");\n"; \
}

#define DEXT1(x, y) \
template<typename T> \
std::string x(int v1) \
{\
    return #y "(" + GET(v1) + ");\n"; \
}

DOP(c_mul, *);

template<typename T, typename U>
std::string c_idiv(int v1, int v2, value_stack& stack_offset, U rtype)
{
    int next = stack_offset.get_next();

    return "if(" + get_variable_name(v2) + " == 0){assert(false);}\n" + types::friendly(rtype) + " " + get_variable_name(next) + " = " + GET(v1) + " / " + GET(v2) + ";\n";
}

DOP(c_fdiv, /);

template<typename T, typename U>
std::string c_remi(int v1, int v2, value_stack& stack_offset, U rtype)
{
    int next = stack_offset.get_next();

    return "if(" + get_variable_name(v2) + " == 0){assert(false);}\n" + types::friendly(rtype) + " " + get_variable_name(next) + " = " + GET(v1) + " % " + GET(v2) + ";\n";
}

DEXT(c_remf, fmod);
DOP(c_iand, &);
DOP(c_ior, |);
DOP(c_ixor, ^);
DOP(c_ishl, <<);
DOP(c_ishr_u, >>);
DOP(c_ishr_s, >>);

template<typename T>
std::string c_irotl(int v1, int v2)
{
    return "(" + GET(v1) + " << " + GET(v2) + ")|(" + GET(v1) + " >> " + std::to_string(sizeof(T)*8) + " - " + GET(v2) + ")" + ";\n";
}

template<typename T>
std::string c_irotr(int v1, int v2)
{
    return "(" + GET(v1) + " >> " + GET(v2) + ")|(" + GET(v1) + " << " + std::to_string(sizeof(T)*8) + " - " + GET(v2) + ")" + ";\n";
}

DEXT1(c_iclz, __builtin_clz);
DEXT1(c_ictz, __builtin_ctz);
DEXT1(c_ipopcnt, __builtin_popcount);

DEXT(c_fmin, std::min);
DEXT(c_fmax, std::max);

DEXT(c_fcopysign, copysign);
DEXT1(c_absf, fabs);
DEXT1(c_fneg, -);
DEXT1(c_fsqrt, sqrt);
DEXT1(c_fceil, ceil);
DEXT1(c_ffloor, floor);
DEXT1(c_ftrunc, trunc);
DEXT1(c_fnearest, nearbyint);

template<typename T, int N>
std::string c_wrap(int v1)
{
    return GET(v1) + " % ((" + types::friendly(T()) + ")pow(2, " + std::to_string(N) + "));\n";
}

template<typename T, typename U>
std::string all_cv(int v1)
{
    return "(" + types::friendly(T()) + ")" + GET(v1) + ";\n";
}

#define C_ALIAS(x) \
template<typename T, typename U> \
std::string x(int v1) \
{ \
    return all_cv<T, U>(v1); \
}

C_ALIAS(c_extend_u);
C_ALIAS(c_extend_s);
C_ALIAS(c_trunc_u);
C_ALIAS(c_trunc_s);
C_ALIAS(c_promote);
C_ALIAS(c_demote);
C_ALIAS(c_convert_u);
C_ALIAS(c_convert_s);

template<typename T, typename U>
std::string c_reinterpret(int v1)
{
    return "*(" + types::friendly(T()) + "*)&" + get_variable_name(v1) + ";\n";
}

/*
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
}*/



#endif // C_BASIC_OPS_HPP_INCLUDED
