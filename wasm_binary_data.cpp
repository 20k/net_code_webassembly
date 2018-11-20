#include "wasm_binary_data.hpp"

#include <array>
#include <iostream>
#include "types.hpp"
#include "LEB.hpp"
#include <assert.h>
#include "serialisable.hpp"
#include "print.hpp"

namespace sections
{
    struct section_header : serialisable
    {
        uint8_t id = 0;
        types::u32 len{0};

        virtual void handle_serialise(parser& p, bool ser) override
        {
            serialise(id, p, ser);
            serialise(len, p, ser);

            std::cout << "found id " << std::to_string(id) << std::endl;
        }
    };

    struct section : serialisable
    {
        section_header header;

        section(const section_header& head) : header(head){}
    };

    struct custom : section
    {
        custom(const section_header& head) : section(head){}

        virtual void handle_serialise(parser& p, bool ser) override
        {
            p.advance((uint32_t)header.len);
        }
    };

    struct type : section
    {
        type(const section_header& head) : section(head){}

        types::vec<types::func> types;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 1)
            {
                throw std::runtime_error("Expected 1, got " + std::to_string(header.id));
            }

            serialise(types, p, ser);
        }
    };

    struct importdesc : serialisable
    {
        types::typeidx tidx;
        types::tabletype tt;
        types::memtype mt;
        types::globaltype gt;

        virtual void handle_serialise(parser& p, bool ser)
        {
            p.checked_fetch<1>({0x00});
            serialise(tidx, p, ser);

            p.checked_fetch<1>({0x01});
            serialise(tt, p, ser);

            p.checked_fetch<1>({0x02});
            serialise(mt, p, ser);

            p.checked_fetch<1>({0x03});
            serialise(gt, p, ser);
        }
    };

    struct import : serialisable
    {
        types::name mod;
        types::name nm;
        importdesc desc;

        virtual void handle_serialise(parser& p, bool ser)
        {
            serialise(mod, p, ser);
            serialise(nm, p, ser);
            serialise(desc, p, ser);
        }
    };

    struct importsec : section
    {
        importsec(const section_header& head) : section(head){}

        types::vec<import> imports;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 2)
            {
                throw std::runtime_error("Expected 2, got " + std::to_string(header.id));
            }

            serialise(imports, p, ser);
        }
    };
}

template<typename T>
T get_section_ignore_custom(parser& p)
{
    sections::section_header head;

    serialise(head, p, false);

    if(head.id == 0)
    {
        sections::custom cust(head);
        serialise(cust, p, false);

        serialise(head, p, false);
    }

    T ret(head);
    serialise(ret, p, false);

    return ret;
}

void wasm_binary_data::init(data d)
{
    parser p(d);

    p.checked_fetch<4>({0x00, 0x61, 0x73, 0x6D});
    p.checked_fetch<4>({0x01, 0x00, 0x00, 0x00});

    for(int i=p.offset; i < 100 && i < (int)p.ptr.size(); i++)
    {
        std::cout << "0x";
        dump(p.ptr[i]);
        std::cout << ", ";
    }

    std::cout << std::dec;

    sections::type stype = get_section_ignore_custom<sections::type>(p);

    std::cout << "num functions " << std::to_string(stype.types.size()) << std::endl;

    for(auto& i : stype.types)
    {
        std::cout << "params " << i.params.size() << std::endl;

        for(auto& p : i.params)
        {
            std::cout << p.friendly() << std::endl;
        }

        std::cout << "results " << i.results.size() << std::endl;

        for(auto& r : i.results)
        {
            std::cout << r.friendly() << std::endl;
        }
    }
}
