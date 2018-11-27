#ifndef RUNTIME_TYPES_HPP_INCLUDED
#define RUNTIME_TYPES_HPP_INCLUDED

#include "types.hpp"
#include <optional>
#include <functional>

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

    struct value;

    struct host_func
    {
        //void(*ptr)(void);
        std::function<std::optional<runtime::value>()> ptr;
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
    ///using a union is not better apparently
    struct value
    {
        std::variant<types::i32, types::i64, types::f32, types::f64> v;

        void from_valtype(const types::valtype& type)
        {
            switch(type.which)
            {
                case 0x7F:
                    v = types::i32{0};
                    return;
                case 0x7E:
                    v = types::i64{0};
                    return;
                case 0x7D:
                    v = types::f32{0};
                    return;
                case 0x7C:
                    v = types::f64{0};
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

        value() = default;

        void set(uint32_t t)
        {
            v = types::i32{t};
        }

        void set(int32_t t)
        {
            v = types::i32{(uint32_t)t};
        }

        void set(uint64_t t)
        {
            v = types::i64{t};
        }

        void set(int64_t t)
        {
            v = types::i64{(uint64_t)t};
        }

        void set(float t)
        {
            v = types::f32{t};
        }

        void set(double t)
        {
            v = types::f64{t};
        }

        void set(const types::i32& in)
        {
            v = in;
        }

        void set(const types::i64& in)
        {
            v = in;
        }

        void set(const types::f32& in)
        {
            v = in;
        }

        void set(const types::f64& in)
        {
            v = in;
        }

        template<typename T>
        auto apply(const T& t)
        {
            if(std::holds_alternative<types::i32>(v))
                return t(std::get<types::i32>(v).val);
            else if(std::holds_alternative<types::i64>(v))
                return t(std::get<types::i64>(v).val);
            else if(std::holds_alternative<types::f32>(v))
                return t(std::get<types::f32>(v).val);
            else if(std::holds_alternative<types::f64>(v))
                return t(std::get<types::f64>(v).val);

            throw std::runtime_error("nope");
        }

        std::string friendly_val()
        {
            return apply([](auto concrete){return std::to_string(concrete);});
        }

        bool is_i32()
        {
            return std::holds_alternative<types::i32>(v);
        }
    };

    inline
    bool same_type(const value& v1, const value& v2)
    {
        if(std::holds_alternative<types::i32>(v1.v) && std::holds_alternative<types::i32>(v2.v))
            return true;
        if(std::holds_alternative<types::i64>(v1.v) && std::holds_alternative<types::i64>(v2.v))
            return true;
        if(std::holds_alternative<types::f32>(v1.v) && std::holds_alternative<types::f32>(v2.v))
            return true;
        if(std::holds_alternative<types::f64>(v1.v) && std::holds_alternative<types::f64>(v2.v))
            return true;

        return false;
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u, const value& v)
    {
        if(!same_type(u, v))
            throw std::runtime_error("Not same type");

        if(std::holds_alternative<types::i32>(u.v))
            return t(std::get<types::i32>(u.v).val, std::get<types::i32>(v.v).val);

        else if(std::holds_alternative<types::i64>(u.v))
            return t(std::get<types::i64>(u.v).val, std::get<types::i64>(v.v).val);

        else if(std::holds_alternative<types::f32>(u.v))
            return t(std::get<types::f32>(u.v).val, std::get<types::f32>(v.v).val);

        else if(std::holds_alternative<types::f64>(u.v))
            return t(std::get<types::f64>(u.v).val, std::get<types::f64>(v.v).val);

        else
            throw std::runtime_error("apply bad type");

        //return t(u, v);
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u)
    {
        if(std::holds_alternative<types::i32>(u.v))
            return t(std::get<types::i32>(u.v).val);

        else if(std::holds_alternative<types::i64>(u.v))
            return t(std::get<types::i64>(u.v).val);

        else if(std::holds_alternative<types::f32>(u.v))
            return t(std::get<types::f32>(u.v).val);

        else if(std::holds_alternative<types::f64>(u.v))
            return t(std::get<types::f64>(u.v).val);

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

        funcaddr allocfunction(const module& m, size_t idx);
        funcaddr allochostfunction(const types::functype& type, const decltype(runtime::host_func::ptr)& ptr);
        ///create a wrapper for allochostfunction which deduces type and automatically creates a shim

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

    constexpr size_t page_size = 64*1024;
    ///128 MB
    constexpr size_t sandbox_mem_cap = 128 * 1024 * 1024;
}

#endif // RUNTIME_TYPES_HPP_INCLUDED
