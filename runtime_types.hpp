#ifndef RUNTIME_TYPES_HPP_INCLUDED
#define RUNTIME_TYPES_HPP_INCLUDED

#include "types.hpp"

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
        types::vec<types::func> typel;

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
        types::func type;
        moduleinst module;

        std::variant<host_func, webasm_func> funct;
    };
}

#endif // RUNTIME_TYPES_HPP_INCLUDED
