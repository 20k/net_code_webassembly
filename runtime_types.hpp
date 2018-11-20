#ifndef RUNTIME_TYPES_HPP_INCLUDED
#define RUNTIME_TYPES_HPP_INCLUDED

#include "types.hpp"
#include <optional>

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

    struct moduleinst
    {
        types::vec<types::functype> typel;

        types::vec<funcaddr> funcaddrs;
        types::vec<tableaddr> tableaddrs;
        types::vec<memaddr> memaddrs;
        types::vec<globaladdr> globaladdrs;

        types::vec<exportinst> exports;
    };

    struct webasm_func
    {
        types::code funct;
    };

    struct host_func
    {
        void(*ptr)(void);
    };

    struct funcinst
    {
        types::functype type;
        moduleinst module;

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

    struct value
    {
        std::variant<types::i32, types::i64, types::f32, types::f64> v;
    };

    struct globalinst
    {
        value val;
        types::mut mut;
    };

    struct store
    {
        types::vec<funcinst> funcs;
        types::vec<tableinst> tables;
        types::vec<meminst> mems;
        types::vec<globalinst> globals;
    };

    template<typename T>
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

    types::vec<funcaddr> filter_func(const types::vec<externval>& vals)
    {
        return filter_type<funcaddr>(vals);
    }

    types::vec<tableaddr> filter_table(const types::vec<externval>& vals)
    {
        return filter_type<tableaddr>(vals);
    }

    types::vec<memaddr> filter_mem(const types::vec<externval>& vals)
    {
        return filter_type<memaddr>(vals);
    }

    types::vec<globaladdr> filter_global(const types::vec<externval>& vals)
    {
        return filter_type<globaladdr>(vals);
    }
}

#endif // RUNTIME_TYPES_HPP_INCLUDED
