#include "compile.hpp"
#include <fstream>

std::string read_file_bin(const std::string& file)
{
    std::ifstream t(file, std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    if(!t.good())
        throw std::runtime_error("Could not open file " + file);

    return str;
}

bool file_exists(const std::string& name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

compile_result compile(const std::string& file)
{
    auto it = file.find_last_of('.');

    if(it == std::string::npos)
    {
        printf("No file extension?");
        throw std::runtime_error("nope");
    }

    std::string stripped(file.begin(), file.begin() + it);

    system(("webassembly_frontend.exe " + file + " > " + file + ".txt 2>&1").c_str());

    compile_result cres;

    if(file_exists(stripped + ".wasm"))
        cres.data = read_file_bin(stripped + ".wasm");

    if(file_exists(file + ".txt"))
        cres.err = read_file_bin(file + ".txt");

    remove((file + ".txt").c_str());
    remove((stripped + ".wasm").c_str());

    return cres;
}
