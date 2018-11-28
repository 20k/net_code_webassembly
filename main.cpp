#include <iostream>
#include "LEB.hpp"
#include "wasm_binary_data.hpp"
#include <utility>

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

///ok so i need to find some way to invoke test_simple_params
///maybe i could make a test_host_func which takes test simple params as an arg, which saves template parameters
///then we pass a pointer to that new test host func into the thing, which gets invoked and then unpacks the vector arguments
///and handles result type?

/*template<int N=0, typename... T>
void do_for(std::tuple<T...>& tup, const types::vec<runtime::value>& u)
{
    using ftype = std::tuple_element_t<N, std::tuple<T...>>;

    if constexpr(N >= sizeof...(T))
        return;

    std::get<N>(tup) = (uint32_t)u[N];

    if constexpr(N+1 >= sizeof...(T))
        return;

    do_for<N+1, T...>(tup, u);
}*/

template<typename tup, std::size_t... Is>
void apply_tup(tup& t, const types::vec<runtime::value>& v, std::index_sequence<Is...>)
{
    ((std::get<Is>(t) = (std::tuple_element_t<Is, tup>)v.get<Is>()), ...);
}

template<typename return_type, typename... args_type> std::optional<runtime::value> host_shim(const types::vec<runtime::value>& vals)
{
    constexpr int num_ppack = sizeof...(args_type);

    std::tuple<args_type...> args;

    //do_for(args, vals);

    std::index_sequence_for<args_type...> iseq;

    apply_tup(args, vals, iseq);
}

int32_t test_simple_params(int32_t v1)
{
    printf("simple params %i\n", v1);

    return 99;
}

int main()
{
    leb_tests();

    data example;
    example.load_from_file("test_1.wasm");

    //runtime::externval tv;
    //tv.val = runtime::funcaddr{0};

    auto shim = &host_shim<int, uint32_t>;

    wasm_binary_data test;

    types::valtype rtype;
    rtype.set<types::i32>();

    /*types::functype ftype;
    ftype.results.push_back(rtype);
    ftype.params.push_back(rtype);*/

    types::functype ftype = get_functype(&test_simple_params);

    runtime::externval tv;
    tv.val = test.s.allochostfunction(ftype, test_host_func);
    ///so it looks like allocfunc gets the typeidx
    ///typeidx goes to functype
    ///runtime::funcinst then has functype type, and then a custom ptr

    test.init(example, {tv});

    return 0;
}
