#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"

int main()
{
    leb_tests();

    data example;
    example.load_from_file("test_1.wasm");

    wasm_binary_data test;
    test.init(example, {});

    return 0;
}
