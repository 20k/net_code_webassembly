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

    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    std::map<std::string, std::map<std::string, runtime::externval>> vals;

    vals["env"]["needs_import"] = tv;

    test.init(example, vals);

    return 0;
}
