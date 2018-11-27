#include "wasm_binary_data.hpp"

#include <array>
#include <iostream>
#include "types.hpp"
#include "LEB.hpp"
#include <assert.h>
#include "serialisable.hpp"
#include "print.hpp"
#include <cstring>
#include "runtime_types.hpp"
#include "invoke.hpp"
#include "logging.hpp"
#include <SFML/System/Clock.hpp>

void dump_state(parser& p)
{
    for(int i=p.offset; i < 500 && i < (int)p.ptr.size(); i++)
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

            lg::log("found id ", std::to_string(id));
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

        types::name name;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            int pre = p.offset;

            serialise(name, p, ser);

            p.offset = pre;

            int len = (uint32_t)header.len;

            p.advance(len);

            lg::log("custom name ", name.friendly());
        }
    };

    struct type : section
    {
        type(const section_header& head) : section(head){}
        type(){}

        types::vec<types::functype> types;

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

            lg::log("import1 ", mod.friendly());
            lg::log("import2 ", nm.friendly());
            ///so mod is the module
            ///env seems to be environment, eg passive imports
            ///nm is the name, in this example the function hi
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

            lg::log("export ", nm.friendly());
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

    struct start : serialisable
    {
        types::funcidx fidx;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            serialise(fidx, p, ser);
        }
    };

    struct startsec : section
    {
        startsec(const section_header& head) : section(head){}
        startsec(){}

        start st;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 8)
            {
                throw std::runtime_error("Expected 8, got " + std::to_string(header.id));
            }

            serialise(st, p, ser);
        }
    };

    struct elem : serialisable
    {
        types::tableidx tidx;
        types::expr e;
        types::vec<types::funcidx> funcs;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            serialise(tidx, p, ser);
            serialise(e, p, ser);
            serialise(funcs, p, ser);
        }
    };

    struct elemsec : section
    {
        elemsec(const section_header& head) : section(head){}
        elemsec(){}

        types::vec<elem> elems;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 9)
            {
                throw std::runtime_error("Expected 9, got " + std::to_string(header.id));
            }

            serialise(elems, p, ser);
        }
    };

    struct codesec : section
    {
        codesec(const section_header& head) : section(head){}
        codesec(){}

        types::vec<types::code> funcs;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 10)
            {
                throw std::runtime_error("Expected 10, got " + std::to_string(header.id));
            }

            int pre = p.offset;

            serialise(funcs, p, ser);

            int post = p.offset;

            int hlen = (uint32_t)header.len;

            if(post - pre != hlen)
            {
                throw std::runtime_error("Bad parse in code section");
            }
        }
    };

    struct datasec : section
    {
        datasec(const section_header& head) : section(head){}
        datasec(){}

        types::vec<types::dataseg> dats;

        virtual void handle_serialise(parser& p, bool ser) override
        {
            if(header.id != 11)
            {
                throw std::runtime_error("Expected 11, got " + std::to_string(header.id));
            }

            serialise(dats, p, ser);
        }
    };
}

///skips custom segments
sections::section_header get_next_header(parser& p)
{
    sections::section_header head;

    serialise(head, p, false);

    /*if(head.id == 0)
    {
        sections::custom cust(head);
        serialise(cust, p, false);

        serialise(head, p, false);
    }*/

    return head;
}

struct module
{
    sections::type section_type;
    sections::importsec section_imports;
    sections::functionsec section_func;
    sections::tablesec section_table;
    sections::memsec section_memory;
    sections::globalsec section_global;
    sections::exportsec section_export;
    sections::startsec section_start;
    sections::elemsec section_elem;
    sections::codesec section_code;
    sections::datasec section_data;

    void init(parser& p)
    {
        p.checked_fetch<4>({0x00, 0x61, 0x73, 0x6D});
        p.checked_fetch<4>({0x01, 0x00, 0x00, 0x00});

        dump_state(p);

        sections::section_header head;

        int num_encoded = 15;

        int num_3 = 0;

        ///current.y implemented two section types
        for(int i=0; i < num_encoded; i++)
        {
            if(p.finished())
                break;

            head = get_next_header(p);

            std::cout <<" HID " << std::to_string(head.id) << std::endl;

            if(head.id >= num_encoded)
                break;

            if(head.id == 0)
            {
                sections::custom cust(head);
                serialise(cust, p, false);

                std::cout << "found custom\n";
            }
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
            else if(head.id == 7)
            {
                section_export = sections::exportsec(head);
                serialise(section_export, p, false);

                std::cout << "imported exports\n";
            }
            else if(head.id == 8)
            {
                section_start = sections::startsec(head);
                serialise(section_start, p, false);

                std::cout << "warning: imported start untested\n";
            }
            else if(head.id == 9)
            {
                section_elem = sections::elemsec(head);
                serialise(section_elem, p, false);

                std::cout << "warning: imported elem section untested\n";
            }
            else if(head.id == 10)
            {
                section_code = sections::codesec(head);
                serialise(section_code, p, false);

                std::cout << "Imported code section\n";
            }
            else if(head.id == 11)
            {
                section_data = sections::datasec(head);
                serialise(section_data,p, false);

                std::cout << "Imported data section\n";
            }
            else
            {
                break;
            }
        }

        ///unsure of spec in this case
        assert(num_3 == 0 || num_3 == 1);


        std::cout << "finished parsing\n";

        std::cout << "num ftypes " << std::to_string(section_type.types.size()) << std::endl;

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

        std::cout << "num globals " << section_global.globals.size() << std::endl;

        std::cout << "num exports " << section_export.exports.size() << std::endl;

        std::cout << "num code " << section_code.funcs.size() << std::endl;
    }
};

runtime::funcaddr runtime::store::allocfunction(const module& m, size_t idx)
{
    int a = funcs.size();

    if(idx >= m.section_func.types.size())
        throw std::runtime_error("Bad function alloc");

    types::typeidx type_idx = m.section_func.types[idx];

    if((uint32_t)type_idx >= (uint32_t)m.section_type.types.size())
        throw std::runtime_error("Bad type_idx in function alloc");

    types::functype type = m.section_type.types[(uint32_t)type_idx];

    webasm_func wfunc;
    wfunc.funct = m.section_code.funcs[idx];

    runtime::funcinst inst;
    inst.type = type;
    inst.funct = wfunc;

    funcs.push_back(inst);

    return funcaddr{a};
}

runtime::funcaddr runtime::store::allochostfunction(const types::functype& type, const std::function<std::optional<runtime::value>(const types::vec<runtime::value>&)>& ptr)
{
    int a = funcs.size();

    host_func hfunc;
    hfunc.ptr = ptr;

    runtime::funcinst inst;
    inst.type = type;
    inst.funct = hfunc;

    funcs.push_back(inst);

    return funcaddr{a};
}

runtime::tableaddr runtime::store::alloctable(const types::tabletype& type)
{
    int a = tables.size();

    runtime::tableinst inst;

    inst.max = type.lim.get_max();
    inst.elem.resize((uint32_t)type.lim.n);

    tables.push_back(inst);

    return tableaddr{a};
}

runtime::memaddr runtime::store::allocmem(const types::memtype& type)
{
    int a = mems.size();

    runtime::meminst inst;

    ///TODO:
    ///SANDBOXING ALERT
    ///this is simply as per spec but might need tweaking
    inst.max = type.lim.get_max();
    inst.dat.resize(runtime::page_size * (uint32_t)type.lim.n);

    mems.push_back(inst);

    return memaddr{a};
}

runtime::globaladdr runtime::store::allocglobal(const types::globaltype& type, const value& v)
{
    int a = globals.size();

    runtime::globalinst inst;

    inst.mut = type.m;
    inst.val = v;

    globals.push_back(inst);

    return globaladdr{a};
}

///imports are temporarily disabled
///but looks like externvals are first then regulars
runtime::moduleinst build_from_module(module& m, runtime::store& s, const types::vec<runtime::externval>& ext)
{
    runtime::moduleinst inst;

    if(m.section_imports.imports.size() != ext.size())
        throw std::runtime_error("Looking for " + std::to_string(m.section_imports.imports.size()) + " but only received " + std::to_string(ext.size()));

    types::vec<runtime::funcaddr> faddr;

    for(int i=0; i < (int)m.section_func.types.size(); i++)
    {
        faddr.push_back(s.allocfunction(m, i));
    }

    types::vec<runtime::tableaddr> taddr;

    for(int i=0; i < (int)m.section_table.tables.size(); i++)
    {
        taddr.push_back(s.alloctable(m.section_table.tables[i].type));
    }

    types::vec<runtime::memaddr> maddr;

    for(int i=0; i < (int)m.section_memory.mems.size(); i++)
    {
        maddr.push_back(s.allocmem(m.section_memory.mems[i].type));
    }

    types::vec<runtime::globaladdr> gaddr;

    ///TODO: IMPORTED GLOBALS GET PUT INTO A TEMPORARY MINST
    ///IMPORTS ARE REALLY NOT SUPPORTED ATM
    //inst.globaladdrs

    for(int i=0; i < (int)m.section_global.globals.size(); i++)
    {
        types::global& glob = m.section_global.globals[i];

        types::vec<runtime::value> val = eval_with_frame(inst, s, glob.e.i);

        if(val.size() == 1)
        {
            std::cout << "evald global and got " << val[0].friendly_val() << std::endl;

            gaddr.push_back(s.allocglobal(m.section_global.globals[i].type, val[0]));
        }
        else
        {
            throw std::runtime_error("val.size() != 1");
        }
    }

    std::cout << "faddr " << (uint32_t)faddr.size() << std::endl;
    std::cout << "taddr " << (uint32_t)taddr.size() << std::endl;
    std::cout << "maddr " << (uint32_t)maddr.size() << std::endl;
    std::cout << "gaddr " << (uint32_t)gaddr.size() << std::endl;

    ///NOT REALLY CLEAR WHICH WAY ROUND THESE BOYS GO
    auto efaddr = runtime::filter_func(ext).append(faddr);
    auto etaddr = runtime::filter_table(ext).append(taddr);
    auto emaddr = runtime::filter_mem(ext).append(maddr);
    auto egaddr = runtime::filter_global(ext).append(gaddr);

    std::cout << "efaddr " << (uint32_t)efaddr.size() << std::endl;
    std::cout << "etaddr " << (uint32_t)etaddr.size() << std::endl;
    std::cout << "emaddr " << (uint32_t)emaddr.size() << std::endl;
    std::cout << "egaddr " << (uint32_t)egaddr.size() << std::endl;

    inst.globaladdrs = egaddr;

    types::vec<runtime::exportinst> my_exports;

    for(size_t i=0; i < m.section_export.exports.size(); i++)
    {
        sections::export_info& inf = m.section_export.exports[i];

        runtime::exportinst out;

        if(std::holds_alternative<types::funcidx>(inf.desc.vals))
        {
            out.value.val = efaddr[(uint32_t)std::get<types::funcidx>(inf.desc.vals)];
        }

        else if(std::holds_alternative<types::tableidx>(inf.desc.vals))
        {
            out.value.val = etaddr[(uint32_t)std::get<types::tableidx>(inf.desc.vals)];
        }

        else if(std::holds_alternative<types::memidx>(inf.desc.vals))
        {
            out.value.val = emaddr[(uint32_t)std::get<types::memidx>(inf.desc.vals)];
        }

        else if(std::holds_alternative<types::globalidx>(inf.desc.vals))
        {
            out.value.val = egaddr[(uint32_t)std::get<types::globalidx>(inf.desc.vals)];
        }
        else
        {
            throw std::runtime_error("Bad export type");
        }

        out.name = inf.nm;

        std::cout << "ename " << out.name.friendly() << std::endl;

        my_exports.push_back(out);
    }

    for(size_t i=0; i < m.section_elem.elems.size(); i++)
    {
        sections::elem& e = m.section_elem.elems[i];

        types::vec<runtime::value> vals = eval_with_frame(inst, s, e.e.i);

        if(vals.size() != 1)
            throw std::runtime_error("Bad section element size");

        runtime::value val = vals[0];

        if(!val.is_i32())
            throw std::runtime_error("Eval'd section element and did not get i32");

        types::tableidx tidx = e.tidx;

        if((uint32_t)tidx >= (uint32_t)etaddr.size())
            throw std::runtime_error("Bad tidx in section element");

        runtime::tableaddr tdr = etaddr[(uint32_t)tidx];

        uint32_t t_addr_idx = (uint32_t)tdr;

        if(t_addr_idx >= (uint32_t)s.tables.size())
            throw std::runtime_error("Bad t_addr_idx");

        runtime::tableinst& tinst = s.tables[t_addr_idx];

        uint32_t eo = (uint32_t)std::get<types::i32>(val.v);
        uint32_t einit = e.funcs.size();

        ///this check is correct
        if(eo + einit > (uint32_t)tinst.elem.size())
            throw std::runtime_error("Bad tinst");

        for(uint32_t j=0; j < (uint32_t)e.funcs.size(); j++)
        {
            types::funcidx fidx = e.funcs[j];

            uint32_t idx = (uint32_t)fidx;

            ///warning, import
            if(idx >= (uint32_t)efaddr.size())
                throw std::runtime_error("bad efaddr idx");

            tinst.elem[eo + j].addr = efaddr[idx];
        }
    }

    for(size_t i=0; i < m.section_data.dats.size(); i++)
    {
        types::dataseg& data_seg = m.section_data.dats[i];

        types::vec<runtime::value> vals = eval_with_frame(inst, s, data_seg.e.i);

        if(vals.size() != 1 || !vals[0].is_i32())
            throw std::runtime_error("Bad data eval");

        runtime::value val = vals[0];

        types::memidx midx = data_seg.x;

        emaddr.check((uint32_t)midx);

        runtime::memaddr mem_addr = emaddr[(uint32_t)midx];

        s.mems.check((uint32_t)mem_addr);

        runtime::meminst& mem_inst = s.mems[(uint32_t)mem_addr];

        uint32_t do_i = (uint32_t)std::get<types::i32>(val.v);
        uint32_t dend = do_i + data_seg.b.size();

        if(dend > (uint32_t)mem_inst.dat.size())
            throw std::runtime_error("Bad data segment end");

        for(uint32_t j=0; j < (uint32_t)data_seg.b.size(); j++)
        {
            uint8_t byte = data_seg.b[j];

            mem_inst.dat[do_i + j] = byte;
        }
    }

    inst.typel = m.section_type.types;

    inst.funcaddrs = efaddr;
    inst.tableaddrs = etaddr;
    inst.memaddrs = emaddr;
    //inst.globaladdrs = egaddr;

    inst.exports = my_exports;

    std::cout << "added " << inst.exports.size() << " exports" << std::endl;

    return inst;
}

void test_hi(int in)
{
    printf("hi");
}

///ok so
///duktape appears to execute this in 62ms or so
///whereas currently i am taking 700ms, so looking for a factor 10x speedup here
///with optimisations on this only takes 350 which is neat

///inlining might not be possible because the functions are mutually dependent
///try converting to an explicit callstack

///in the latest test, duktape hits 350 but I hit 1800
///currently duktape its 380 on a good run, and I hit 820 on a good run
///now its 700, woop woop
///now down to 675 with label fixes
///660 with other stuff
///650, time to stop probably sadly

///duk = 635
///us = 990
void wasm_binary_data::init(data d, const types::vec<runtime::externval>& eval)
{
    parser p(d);

    module mod;
    mod.init(p);

    ///ideally we'd build this from actually evaluating the expressions for the globals
    ///but we're not there yet
    /*types::vec<runtime::value> global_init;
    for(int i=0; i < 100; i++)
    {
        runtime::value val;
        val.v = types::i32{0};

        global_init.push_back(val);
    }*/

    runtime::moduleinst minst = build_from_module(mod, s, eval);

    m_minst = new runtime::moduleinst(minst);

    //s.invoke({0})

    /*for(int i=0; i < (int)minst.exports.v.size(); i++)
    {
        runtime::exportinst& einst = minst.exports.v[i];

        if(einst.name == "_start")
        {
            std::cout << "hello looking at _start" << std::endl;

            s.invoke(std::get<runtime::funcaddr>(einst.value.val), minst, {});
        }
    }*/

    //types::vec<runtime::value> vals = s.invoke_by_name("_start", minst, {});

    runtime::value arg;
    //arg.set((uint32_t)(13*17));
    arg.set((uint32_t)(15485863));
    //arg.set((uint32_t)(1188179));

    //arg.set({13*17});

    //types::vec<runtime::value> args;
    //args.push_back(arg);

    sf::Clock clk;

    types::vec<runtime::value> vals = s.invoke_by_name("import_test", minst, {arg});
    //types::vec<runtime::value> vals = s.invoke_by_name("call_is_prime", minst, {arg});

    std::cout << "time " << clk.getElapsedTime().asMicroseconds() / 1000. << std::endl;

    //std::cout << "rvals " << vals.size() << std::endl;

    for(runtime::value& val : vals)
    {
        std::cout << "rval " << val.friendly_val() << std::endl;
    }
}
