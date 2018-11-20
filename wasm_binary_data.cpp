#include "wasm_binary_data.hpp"

#include <array>
#include <iostream>
#include "types.hpp"
#include "LEB.hpp"
#include <assert.h>
#include "serialisable.hpp"
#include "print.hpp"
#include <cstring>

void dump_state(parser& p)
{
    for(int i=p.offset; i < 100 && i < (int)p.ptr.size(); i++)
    {
        std::cout << "0x";
        dump(p.ptr[i]);
        std::cout << ", ";
    }

    std::cout << std::dec;
}

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
        section(){}
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
        type(){}

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
        ///0 -> func
        ///1 -> table
        ///2 -> mem
        ///3 -> global
        uint8_t which = 0;

        std::variant<types::typeidx, types::tabletype, types::memtype, types::globaltype> vals;

        virtual void handle_serialise(parser& p, bool ser)
        {
            which = p.next();

            if(ser == false)
            {
                switch(which)
                {
                    case 0x00:
                        lowest_get<types::typeidx>(vals, p);
                        break;
                    case 0x01:
                        lowest_get<types::tabletype>(vals, p);
                        break;
                    case 0x02:
                        lowest_get<types::memtype>(vals, p);
                        break;
                    case 0x03:
                        lowest_get<types::globaltype>(vals, p);
                        break;
                    default:
                        throw std::runtime_error("Invalid which switch in importdesc " + std::to_string(which));
                }
            }
            else
            {
                assert(false);
            }
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
        importsec(){}

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

    struct functionsec : section
    {
        functionsec(const section_header& head) : section(head){}
        functionsec(){}

        types::vec<types::typeidx> types;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 3)
            {
                throw std::runtime_error("Expected 3, got " + std::to_string(header.id));
            }

            serialise(types, p, ser);
        }
    };

    struct tablesec : section
    {
        tablesec(const section_header& head) : section(head){}
        tablesec(){}

        types::vec<types::table> tables;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 4)
            {
                throw std::runtime_error("Expected 4, got " + std::to_string(header.id));
            }

            serialise(tables, p, ser);
        }
    };

    struct memsec : section
    {
        memsec(const section_header& head) : section(head){}
        memsec(){}

        types::vec<types::mem> mems;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 5)
            {
                throw std::runtime_error("Expected 5, got " + std::to_string(header.id));
            }

            serialise(mems, p, ser);
        }
    };

    struct globalsec : section
    {
        globalsec(const section_header& head) : section(head){}
        globalsec(){}

        types::vec<types::global> globals;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 6)
            {
                throw std::runtime_error("Expected 5, got " + std::to_string(header.id));
            }

            serialise(globals, p, ser);
        }
    };

    struct exportdesc : serialisable
    {
        uint8_t which = 0;

        std::variant<types::funcidx, types::tableidx, types::memidx, types::globalidx> vals;

        virtual void handle_serialise(parser& p, bool ser)
        {
            which = p.next();

            if(ser == false)
            {
                switch(which)
                {
                    case 0x00:
                        lowest_get<types::funcidx>(vals, p);
                        break;
                    case 0x01:
                        lowest_get<types::tableidx>(vals, p);
                        break;
                    case 0x02:
                        lowest_get<types::memidx>(vals, p);
                        break;
                    case 0x03:
                        lowest_get<types::globalidx>(vals, p);
                        break;
                    default:
                        throw std::runtime_error("Invalid which switch in importdesc " + std::to_string(which));
                }
            }
            else
            {
                assert(false);
            }
        }
    };

    struct export_info : serialisable
    {
        types::name nm;
        exportdesc desc;

        virtual void handle_serialise(parser& p, bool ser)
        {
            serialise(nm, p, ser);
            serialise(desc, p, ser);
        }
    };

    struct exportsec : section
    {
        exportsec(const section_header& head) : section(head){}
        exportsec(){}

        types::vec<export_info> exports;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 7)
            {
                throw std::runtime_error("Expected 7, got " + std::to_string(header.id));
            }

            serialise(exports, p, ser);
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

    ///a lotta these headers are optional
    ///so really we should just mix and match which one goes where

    T ret(head);

    serialise(ret, p, false);

    return ret;
}

///skips custom segments
sections::section_header get_next_header(parser& p)
{
    sections::section_header head;

    serialise(head, p, false);

    if(head.id == 0)
    {
        sections::custom cust(head);
        serialise(cust, p, false);

        serialise(head, p, false);
    }

    return head;
}

void wasm_binary_data::init(data d)
{
    parser p(d);

    p.checked_fetch<4>({0x00, 0x61, 0x73, 0x6D});
    p.checked_fetch<4>({0x01, 0x00, 0x00, 0x00});

    dump_state(p);

    sections::type section_type;
    sections::importsec section_imports;
    sections::functionsec section_func;
    sections::tablesec section_table;
    sections::memsec section_memory;
    sections::globalsec section_global;

    sections::section_header head;

    int num_encoded = 10;

    int num_3 = 0;

    ///current.y implemented two section types
    for(int i=0; i < num_encoded; i++)
    {
        head = get_next_header(p);

        std::cout <<" HID " << std::to_string(head.id) << std::endl;

        if(head.id >= num_encoded)
            break;

        if(head.id == 0)
            continue;
        else if(head.id == 1)
        {
            //dump_state(p);

            section_type = sections::type(head);
            serialise(section_type, p, false);
            std::cout << "import type\n";
        }
        else if(head.id == 2)
        {
            section_imports = sections::importsec(head);
            serialise(section_imports, p, false);
            std::cout << "import imports\n";
        }
        else if(head.id == 3)
        {
            section_func = sections::functionsec(head);
            serialise(section_func, p, false);
            std::cout << "import functionsec\n";

            num_3++;
        }
        else if(head.id == 4)
        {
            section_table = sections::tablesec(head);
            serialise(section_table, p, false);

            std::cout << "imported table\n";
        }
        else if(head.id == 5)
        {
            section_memory = sections::memsec(head);
            serialise(section_memory, p, false);

            std::cout << "imported memory\n";
        }
        else if(head.id == 6)
        {
            section_global = sections::globalsec(head);
            serialise(section_global, p, false);

            std::cout << "imported global\n";
        }
        else
        {
            break;
        }
    }

    ///unsure of spec in this case
    assert(num_3 == 0 || num_3 == 1);


    std::cout << "finished parsing\n";

    std::cout << "num functions " << std::to_string(section_type.types.size()) << std::endl;

    for(auto& i : section_type.types)
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

    std::cout << "num iports " << section_imports.imports.size() << std::endl;

    std::cout << "num functionsecs " << section_func.types.size() << std::endl;

    std::cout << "num tables " << section_table.tables.size() << std::endl;

    std::cout << "num mem " << section_memory.mems.size() << std::endl;
}
