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

///alright its time for some c++ wizadry now
///work with parameters first, then do return type

int test_simple_params(int v1)
{
    printf("simple params %i\n", v1);

    return 99;
}

template<typename check>
void count_types(int& cnt)
{

}

template<typename check, typename U, typename... T>
void count_types(int& cnt)
{
    if constexpr(std::is_same<check, U>())
    {
        cnt++;

        count_types<check, T...>(cnt);
    }
    else
    {
        return;
    }
}

template<typename T, typename... U> inline types::functype get_params(T(*func)(U... args))
{
    types::functype ret;

    types::valtype i32;
    types::valtype i64;
    types::valtype f32;
    types::valtype f64;

    i32.set<types::i32>();
    i64.set<types::i64>();
    f32.set<types::f32>();
    f64.set<types::f64>();

    int i32_c = 0;
    int i64_c = 0;
    int f32_c = 0;
    int f64_c = 0;

    int i32_r = 0;
    int i64_r = 0;
    int f32_r = 0;
    int f64_r = 0;

    count_types<bool, U...>(i32_c);
    count_types<uint8_t, U...>(i32_c);
    count_types<int8_t, U...>(i32_c);
    count_types<uint16_t, U...>(i32_c);
    count_types<int16_t, U...>(i32_c);
    count_types<uint32_t, U...>(i32_c);
    count_types<int32_t, U...>(i32_c);

    count_types<uint64_t, U...>(i64_c);
    count_types<int64_t, U...>(i64_c);

    count_types<float, U...>(f32_c);

    count_types<double, U...>(f64_c);


    count_types<bool, T>(i32_r);
    count_types<uint8_t, T>(i32_r);
    count_types<int8_t, T>(i32_r);
    count_types<uint16_t, T>(i32_r);
    count_types<int16_t, T>(i32_r);
    count_types<uint32_t, T>(i32_r);
    count_types<int32_t, T>(i32_r);

    count_types<uint64_t, T>(i64_r);
    count_types<int64_t, T>(i64_r);

    count_types<float, T>(f32_r);

    count_types<double, T>(f64_r);


    for(int i=0; i < i32_c; i++)
    {
        ret.params.push_back(i32);
    }

    for(int i=0; i < i64_c; i++)
    {
        ret.params.push_back(i64);
    }

    for(int i=0; i < f32_c; i++)
    {
        ret.params.push_back(f32);
    }

    for(int i=0; i < f64_c; i++)
    {
        ret.params.push_back(f64);
    }


    for(int i=0; i < i32_r; i++)
    {
        ret.results.push_back(i32);
    }

    for(int i=0; i < i64_r; i++)
    {
        ret.results.push_back(i64);
    }

    for(int i=0; i < f32_r; i++)
    {
        ret.results.push_back(f32);
    }

    for(int i=0; i < f64_r; i++)
    {
        ret.results.push_back(f64);
    }

    return ret;
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

    /*types::functype ftype;
    ftype.results.push_back(rtype);
    ftype.params.push_back(rtype);*/

    types::functype ftype = get_params(&test_simple_params);

    runtime::externval tv;
    tv.val = test.s.allochostfunction(ftype, test_host_func);
    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    test.init(example, {tv});

    return 0;
}
