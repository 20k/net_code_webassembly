#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"
#include <utility>
#include <memory>
#include "compile.hpp"

std::optional<runtime::value> test_host_func(const types::vec<runtime::value>& vals, runtime::store& s)
{
    printf("host func!\n");

    std::cout << "num args " << vals.size() << std::endl;

    for(auto& i : vals)
    {
        std::cout << i.friendly_val() << std::endl;
    }

    return runtime::value((uint32_t)12);
}

/*uint32_t test_simple_params(uint32_t v1)
{
    printf("simple params %i\n", v1);

    return 53;
}*/

uint32_t test_simple_params(char* val)
{
    printf("val %s\n", val);

    return 64;
}

void print(const char* ptr)
{
    printf("%s\n", ptr);
}

/*import1 env
import2 _ZSt15get_new_handlerv
import1 env
import2 __syscall1
import1 env
import2 __syscall3
import1 env
import2 __syscall5*/

///just immediately aborts
uint32_t _ZSt15get_new_handlerv()
{
    throw std::runtime_error("Attempt to get new handler!");
}

using stype = uint32_t;

template<typename... T>
uint32_t do_syscall(stype val, T... vals)
{
    throw std::runtime_error("Syscalls not supported");
}

auto syscall0 = do_syscall<>;
auto syscall1 = do_syscall<stype>;
auto syscall2 = do_syscall<stype, stype>;
auto syscall3 = do_syscall<stype, stype, stype>;
auto syscall4 = do_syscall<stype, stype, stype, stype>;
auto syscall5 = do_syscall<stype, stype, stype, stype, stype>;
auto syscall6 = do_syscall<stype, stype, stype, stype, stype, stype>;

///ok so
///js isn't actually js unfortunately, its typescript, and its forcibly typescript which isn't good enough
///so it looks like the 'wasm everything' plan is out of the window

///so, currently i can get pointers to webassembly memory
///and I can get C style structures in and out
///so we need to be able to go js Object -> c structure, and back
int main()
{
    leb_tests();

    data example;
    //example.load_from_file("optimized.wasm");

    compile_result res = compile("sample.cpp");

    std::cout << "ferror " << res.err << std::endl;

    if(res.data.size() == 0)
        return 0;

    example.load_from_data(res.data);

    //runtime::externval tv;
    //tv.val = runtime::funcaddr{0};

    //auto shim = host_shim<test_simple_params>(test_simple_params);

    //auto shim = base_shim<test_simple_params>();

    wasm_binary_data test;

    types::valtype rtype;
    rtype.set<types::i32>();

    /*types::functype ftype;
    ftype.results.push_back(rtype);
    ftype.params.push_back(rtype);*/

    //types::functype ftype = get_functype(&test_simple_params);

    runtime::externval tv;
    //tv.val = test.s.allochostfunction(ftype, test_host_func);
    tv.val = runtime::allochostsimplefunction<test_simple_params>(test.s);

    runtime::externval tprint;
    tprint.val = runtime::allochostsimplefunction<print>(test.s);

    runtime::externval new_handle;
    new_handle.val = runtime::allochostsimplefunction<_ZSt15get_new_handlerv>(test.s);

    runtime::externval s0;
    runtime::externval s1;
    runtime::externval s2;
    runtime::externval s3;
    runtime::externval s4;
    runtime::externval s5;
    runtime::externval s6;

    s0.val = runtime::allochostsimplefunction<syscall0>(test.s);
    s1.val = runtime::allochostsimplefunction<syscall1>(test.s);
    s2.val = runtime::allochostsimplefunction<syscall2>(test.s);
    s3.val = runtime::allochostsimplefunction<syscall3>(test.s);
    s4.val = runtime::allochostsimplefunction<syscall4>(test.s);
    s5.val = runtime::allochostsimplefunction<syscall5>(test.s);
    s6.val = runtime::allochostsimplefunction<syscall6>(test.s);


    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    std::map<std::string, std::map<std::string, runtime::externval>> vals;

    vals["env"]["needs_import"] = tv;
    vals["env"]["print"] = tprint;
    vals["env"]["_ZSt15get_new_handlerv"] = new_handle;

    vals["env"]["__syscall0"] = s0;
    vals["env"]["__syscall1"] = s1;
    vals["env"]["__syscall2"] = s2;
    vals["env"]["__syscall3"] = s3;
    vals["env"]["__syscall4"] = s4;
    vals["env"]["__syscall5"] = s5;
    vals["env"]["__syscall6"] = s6;

    test.init(example, vals);

    return 0;
}
