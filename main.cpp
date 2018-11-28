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

template<typename F, typename Tuple, size_t ...S >
decltype(auto) apply_tuple_impl(F&& fn, Tuple&& t, std::index_sequence<S...>)
{
    return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
}

template<typename F, typename Tuple>
decltype(auto) apply_from_tuple(F&& fn, Tuple&& t)
{
    std::size_t constexpr tSize
        = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
    return apply_tuple_impl(std::forward<F>(fn),
                            std::forward<Tuple>(t),
                            std::make_index_sequence<tSize>());
}

template<typename tup, std::size_t... Is>
void fix_tup(tup& t, const types::vec<runtime::value>& v, std::index_sequence<Is...>)
{
    ((std::get<Is>(t) = (std::tuple_element_t<Is, tup>)v.get<Is>()), ...);
}

template<typename V, const V& v, typename return_type, typename... args_type>
std::optional<runtime::value> host_shim(const types::vec<runtime::value>& vals)
{
    constexpr int num_ppack = sizeof...(args_type);

    std::tuple<args_type...> args;

    std::index_sequence_for<args_type...> iseq;

    fix_tup(args, vals, iseq);

    if constexpr(std::is_same_v<return_type, void>)
    {
        apply_from_tuple(v, args);
        return std::nullopt;
    }

    return apply_from_tuple(v, args);
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

    auto shim = &host_shim<decltype(test_simple_params), test_simple_params, int, uint32_t>;

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
