#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"

std::optional<runtime::value> test_host_func(const types::vec<runtime::value>& vals)
{
    printf("host func!\n");

    std::cout << "num args " << vals.size() << std::endl;

    for(auto& i : vals)
    {
        std::cout << i.friendly_val() << std::endl;
    }

    return runtime::value((uint32_t)12);
}

int main()
{
    leb_tests();

    data example;
    example.load_from_file("test_1.wasm");

    //runtime::externval tv;
    //tv.val = runtime::funcaddr{0};

    wasm_binary_data test;

    types::valtype rtype;
    rtype.set<types::i32>();

    types::functype ftype;
    ftype.results.push_back(rtype);
    ftype.params.push_back(rtype);

    runtime::externval tv;
    tv.val = test.s.allochostfunction(ftype, test_host_func);
    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    test.init(example, {tv});

    return 0;
}
