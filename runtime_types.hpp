#ifndef RUNTIME_TYPES_HPP_INCLUDED
#define RUNTIME_TYPES_HPP_INCLUDED

#include "types.hpp"
#include <optional>
#include <type_traits>

struct module;

namespace runtime
{
    struct addr : types::integral<uint32_t, addr>{};

    struct funcaddr : addr {};;
    struct tableaddr : addr {};
    struct memaddr : addr {};
    struct globaladdr : addr {};

    struct externval
    {
        std::variant<funcaddr, tableaddr, memaddr, globaladdr> val;
    };

    struct exportinst
    {
        types::name name;
        externval value;
    };

    struct webasm_func
    {
        types::code funct;
    };

    struct host_func
    {
        void(*ptr)(void);
    };

    ///uuh
    ///this probably should be a pointer or something to moduleinst
    struct funcinst
    {
        types::functype type;
        //moduleinst module;
        ///do this implicitly at runtime?

        std::variant<host_func, webasm_func> funct;
    };

    struct funcelem
    {
        std::optional<funcaddr> addr;
    };

    struct tableinst
    {
        types::vec<funcelem> elem;
        std::optional<types::u32> max;
    };

    struct meminst
    {
        types::vec<uint8_t> dat;
        std::optional<types::u32> max;
    };

    ///hmm
    ///might be better to hold a uint64_t
    ///and then just cast on the fly
    struct value
    {
        //std::variant<types::i32, types::i64, types::f32, types::f64> v;

        uint8_t wch = 0;

        union un
        {
            uint32_t i_32;
            uint64_t i_64;
            float f_32;
            double f_64;

            un()
            {
                //memset(this, 0, sizeof(un));
            }
        } s;

        void from_valtype(const types::valtype& type)
        {
            switch(type.which)
            {
                case 0x7F:
                    //v = types::i32{0};
                    wch = 1;
                    s.i_32 = 0;
                    return;
                case 0x7E:
                    //v = types::i64{0};
                    wch = 2;
                    s.i_64 = 0;
                    return;
                case 0x7D:
                    //v = types::f32{0};
                    wch = 3;
                    s.f_32 = 0;
                    return;
                case 0x7C:
                    //v = types::f64{0};
                    wch = 4;
                    s.f_64 = 0;
                    return;
                default:
                    throw std::runtime_error("Invalid value in which");
            }
        }

        template<typename T>
        value(const T& in)
        {
            set(in);
        }

        value() {}

        //value(){memset((char*)&s, 0, sizeof(s));};

        value(const runtime::value& val)
        {
            wch = val.wch;
            s = val.s;
        }

        /*void set(const runtime::value& val)
        {
            which = val.which;
            s = val.s;
        }*/

        void set(uint32_t t)
        {
            s.i_32 = t;
            wch = 1;
        }

        void set(int32_t t)
        {
            s.i_32 = (uint32_t)t;
            wch = 1;
        }

        void set(uint64_t t)
        {
            s.i_64 = t;
            wch = 2;
        }

        void set(int64_t t)
        {
            s.i_64 = (uint64_t)t;
            wch = 2;
        }

        void set(float t)
        {
            s.f_32 = t;
            wch = 3;
        }

        void set(double t)
        {
            s.f_64 = t;
            wch = 4;
        }

        template<typename T>
        static bool holds_alternative(const value& in)
        {
            if(std::is_same_v<T, types::i32> && in.wch == 1)
                return true;

            if(std::is_same_v<T, types::i64> && in.wch == 2)
                return true;

            if(std::is_same_v<T, types::f32> && in.wch == 3)
                return true;

            if(std::is_same_v<T, types::f64> && in.wch == 4)
                return true;

            return false;
        }

        template<typename T>
        static decltype(T::val) get(const value& in)
        {
            if constexpr(std::is_same_v<T, types::i32>)
                return in.s.i_32;

            if constexpr(std::is_same_v<T, types::i64>)
                return in.s.i_64;

            if constexpr(std::is_same_v<T, types::f32>)
                return in.s.f_32;

            if constexpr(std::is_same_v<T, types::f64>)
                return in.s.f_64;
        }

        template<typename T>
        auto apply(const T& t)
        {
            if(holds_alternative<types::i32>(*this))
                return t(get<types::i32>(*this));
            else if(holds_alternative<types::i64>(*this))
                return t(get<types::i64>(*this));
            else if(holds_alternative<types::f32>(*this))
                return t(get<types::f32>(*this));
            else if(holds_alternative<types::f64>(*this))
                return t(get<types::f64>(*this));

            throw std::runtime_error("nope");
        }

        std::string friendly_val()
        {
            return apply([](auto concrete){return std::to_string(concrete);});
        }

        bool is_i32()
        {
            return holds_alternative<types::i32>(*this);
        }
    };

    inline
    bool same_type(const value& v1, const value& v2)
    {
        if(value::holds_alternative<types::i32>(v1) && value::holds_alternative<types::i32>(v2))
            return true;
        if(value::holds_alternative<types::i64>(v1) && value::holds_alternative<types::i64>(v2))
            return true;
        if(value::holds_alternative<types::f32>(v1) && value::holds_alternative<types::f32>(v2))
            return true;
        if(value::holds_alternative<types::f64>(v1) && value::holds_alternative<types::f64>(v2))
            return true;

        return false;
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u, const value& v)
    {
        if(!same_type(u, v))
            throw std::runtime_error("Not same type");

        if(value::holds_alternative<types::i32>(u))
            return t(value::get<types::i32>(u), value::get<types::i32>(v));

        else if(value::holds_alternative<types::i64>(u))
            return t(value::get<types::i64>(u), value::get<types::i64>(v));

        else if(value::holds_alternative<types::f32>(u))
            return t(value::get<types::f32>(u), value::get<types::f32>(v));

        else if(value::holds_alternative<types::f64>(u))
            return t(value::get<types::f64>(u), value::get<types::f64>(v));

        else
            throw std::runtime_error("apply bad type");

        //return t(u, v);
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u)
    {
        if(value::holds_alternative<types::i32>(u))
            return t(value::get<types::i32>(u));

        else if(value::holds_alternative<types::i64>(u))
            return t(value::get<types::i64>(u));

        else if(value::holds_alternative<types::f32>(u))
            return t(value::get<types::f32>(u));

        else if(value::holds_alternative<types::f64>(u))
            return t(value::get<types::f64>(u));

        else
            throw std::runtime_error("apply bad type");

        //return t(u, v);
    }

    struct globalinst
    {
        value val;
        types::mut mut;
    };

    struct moduleinst;

    struct store
    {
        types::vec<funcinst> funcs;
        types::vec<tableinst> tables;
        types::vec<meminst> mems;
        types::vec<globalinst> globals;

        funcaddr allocfunction(const module& m, int idx);
        funcaddr allochostfunction(const types::functype& type, void(*ptr)());

        tableaddr alloctable(const types::tabletype& type);
        memaddr allocmem(const types::memtype& type);
        globaladdr allocglobal(const types::globaltype& type, const value& v);

        types::vec<runtime::value> invoke(const funcaddr& address, moduleinst& minst, const types::vec<value>& vals);
        types::vec<runtime::value> invoke_by_name(const std::string& imported, moduleinst& minst, const types::vec<value>& vals);
    };

    template<typename T>
    inline
    types::vec<T> filter_type(const types::vec<externval>& vals)
    {
        types::vec<T> ret;

        for(const externval& val : vals)
        {
            if(std::holds_alternative<T>(val.val))
            {
                ret.push_back(std::get<T>(val.val));
            }
        }

        return ret;
    }

    inline
    types::vec<funcaddr> filter_func(const types::vec<externval>& vals)
    {
        return filter_type<funcaddr>(vals);
    }

    inline
    types::vec<tableaddr> filter_table(const types::vec<externval>& vals)
    {
        return filter_type<tableaddr>(vals);
    }

    inline
    types::vec<memaddr> filter_mem(const types::vec<externval>& vals)
    {
        return filter_type<memaddr>(vals);
    }

    inline
    types::vec<globaladdr> filter_global(const types::vec<externval>& vals)
    {
        return filter_type<globaladdr>(vals);
    }

    ///so this is constructed from our module
    ///which is the section representation we constructed earlier
    struct moduleinst
    {
        types::vec<types::functype> typel;

        types::vec<funcaddr> funcaddrs;
        types::vec<tableaddr> tableaddrs;
        types::vec<memaddr> memaddrs;
        types::vec<globaladdr> globaladdrs;

        types::vec<exportinst> exports;
    };
}

#endif // RUNTIME_TYPES_HPP_INCLUDED
