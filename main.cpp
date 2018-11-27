#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"

int main()
{
    leb_tests();

    data example;
    example.load_from_file("test_1.wasm");

    runtime::externval tv;
    tv.val = runtime::funcaddr{0};
    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    wasm_binary_data test;
    test.init(example, {tv});

    return 0;
}
