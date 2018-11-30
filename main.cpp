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

std::string capture_exec(const std::string& cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while(!feof(pipe.get()))
    {
        if(fgets(buffer.data(), 128, pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }

    return result;
}

/*std::string compile(const std::string& file)
{
    auto it = file.find_last_of('.');

    if(it == std::string::npos)
    {
        printf("No file extension?");
        throw std::runtime_error("nope");
    }

    std::string stripped(file.begin(), file.begin() + it);

    //system(("start ./frontend/webassembly_frontend.exe " + file).c_str());

    std::cout << capture_exec("start ./frontend/webassembly_frontend.exe " + file) << std::endl;

    return read_file_bin(stripped + ".wasm");
}*/

int main()
{
    leb_tests();

    data example;
    //example.load_from_file("optimized.wasm");

    example.load_from_data(compile("sample.cpp"));

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
