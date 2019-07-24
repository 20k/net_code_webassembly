#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"
#include <utility>
#include <memory>
#include "compile.hpp"
#include "injection_helpers.hpp"
#include "JIT.hpp"
#include "c_backend.hpp"
#include <SFML/System/Clock.hpp>

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
    printf("pre %s post\n", ptr);
}

///ok so
///js isn't actually js unfortunately, its typescript, and its forcibly typescript which isn't good enough
///so it looks like the 'wasm everything' plan is out of the window

///so, currently i can get pointers to webassembly memory
///and I can get C style structures in and out
///so we need to be able to go js Object -> c structure, and back
int main()
{
    //test_jit();
    leb_tests();

    data example;
    //example.load_from_file("optimized.wasm");

    /*compile_result res = compile("sample.cpp");

    std::cout << "ferror " << res.err << std::endl;

    if(res.data.size() == 0)
        return 0;

    //example.load_from_data(res.data);*/

    example.load_from_file("test.m");


    wasm_binary_data test;

    runtime::externval tv;
    tv.val = runtime::allochostsimplefunction<test_simple_params>(test.s);

    runtime::externval tprint;
    tprint.val = runtime::allochostsimplefunction<print>(test.s);

    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    ///Ok optimisation level is changing stuff, so basically the verdict is that the impl difference is UB
    ///yay..
    ///l1 x86_64-w64-mingw32-g++.exe -Wall -fexceptions -std=c++17 -Wno-attributes -O2  -c ./out.cpp -o out.o
    ///l2 x86_64-w64-mingw32-g++.exe  -o sampler.exe out.o  -s
    ///l3 g++ out.cpp -o test_wasm -Wno-attributes -std=c++17


    std::map<std::string, std::map<std::string, runtime::externval>> vals = get_env_helpers(test.s);

    vals["env"]["needs_import"] = tv;
    vals["env"]["print"] = tprint;

    test.init(example, vals);
    //test.self_test();
    //return 0;

    wasm_binary_data exec = test;

    exec.m_minst = new runtime::moduleinst(*exec.m_minst);
    //exec.self_test();

    std::string as_c_program = get_as_c_program(test);

    if(as_c_program.size() > 0)
    {
        auto file = std::fstream("out.cpp", std::ios::out | std::ios::binary);
        file.write(as_c_program.c_str(), as_c_program.size());
    }

    return 0;
}
