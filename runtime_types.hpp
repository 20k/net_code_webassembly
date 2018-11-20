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

        types::vec<types::funcaddr> funcaddrs
        types::vec<types::tableaddr> tableaddrs
        types::vec<types::memaddr> memaddrs
        types::vec<types::globaladdr> globaladdrs

        types::vec<exportinst> exports;
    };
}

#endif // RUNTIME_TYPES_HPP_INCLUDED
