#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <map>
//#include "gameapi.hpp"

#define WASM_EXPORT __attribute__ ((visibility ("default"), used, noinline)) extern "C"

//extern "C" int needs_import(const char* x);
extern "C" void print(const char* x);

///yeah so misnomer
WASM_EXPORT
int needs_import(const char* x)
{
    return strlen(x);
}

WASM_EXPORT
int main();

///how on earth are function pointers going to work?
///obviously itll have to be only c style pointers, but even then...
///maybe they'll take a vector of serialisables and return an optional to a serialisable (or a game_api_t which we can treat a bit like JSON?)?
///maybe it should be a vector of game_api_ts and return an optional, or game_api_t -1 should be an error for glorious c compat
///it'd be easier to take one gameapi_t and then dynamically unpack it through template magic?

#if 0
struct test_subclass : serialisable
{
    std::string tval;

    SERIALISE_FUNC()
    {
        SER(tval);
    }
};

struct test_serialisable : serialisable
{
    std::string val_1;
    std::string val_2;
    uint32_t test_int;
    test_subclass tsub;

    /*game_api_t raw_function(game_api_t args)
    {
        return 12;
    }*/

    ///cpp member functions need pointer transfer before i can do other stuff?

    static game_api_t raw_function(game_api_t args)
    {
        return 53;
    }

    game_func fptr = &raw_function;

    SERIALISE_FUNC()
    {
        SER(val_1);
        SER(val_2);
        SER(test_int);
        SER(tsub);
        SER(fptr);
        //SER(&test_serialisable::raw_function);
    }
};

///hmm so there's a suspicious constant
///r2 loads 1868920584 where r1 loads 8
///need to grab the binary payload

///TODO, strings across the boundary
WASM_EXPORT
void test_serialise()
{
    //object passthrough;

    test_serialisable test;
    test.val_1 = "asdf";
    test.val_2 = "second";
    test.test_int = 53;
    test.tsub.tval = "some_tval";

    //c_str nullkey("");

    game_api_t gapi = get_next_id();
    serialise_root(gapi, test, true);

    test_serialisable test_de;
    serialise_root(gapi, test_de, false);

    //std::string sdfdf = std::to_string(test_de.test_int);

    std::string sdfdf = test_de.val_1;

    //print(sdfdf.c_str());

    print(test_de.tsub.tval.c_str());

    /*serialise_object_begin_base(gapi);
    to_gameapi(gapi, test, "test", true);
    serialise_object_end_base(gapi);*/

    //serialise(passthrough, test, "", true);

    /*std::map<std::string, std::string> rmap;

    rmap["test"] = std::string("weow");

    std::string fstr = rmap["test"];

    if(fstr == "")
        print(fstr.c_str());*/

    //std::map<std::string, std::string> rmap{{"weow", std::string("test")}};

    /*std::map<std::string, std::string> rmap;

    rmap["weow"] = std::string("asdfdsf");

    if(rmap["weow"] == "")
        print(rmap["weow"].c_str());*/

    /*std::string some_test = "asdf43212345";

    print(&some_test[5]);*/
}
#endif

WASM_EXPORT
int import_test(int x)
{
    return needs_import("hello");
}

int func_2(int x)
{
    //FILE* pFile = fopen("test.txt", "w");

    return 2;
}

WASM_EXPORT
int capi_test()
{
    char test[5] = {1,2,3,4,0};

    char* str = (char*)malloc(12);

    memset(str, 0, 12);

    str[0] = 'p';
    str[1] = 'o';
    str[2] = 'o';
    str[3] = 'p';

    return strlen(str);
}

WASM_EXPORT
int arg_test()
{
    int v1 = 1;
    int v2 = 2;

    return v1 - v2;
}

void _start()
{
    main();
}

WASM_EXPORT
int undefined_test(unsigned int x)
{
    /*void* ptr = malloc(10);

    return (int)ptr;*/

    return 0;
}

WASM_EXPORT
int is_prime(unsigned int x) {
  unsigned int divisor = 1;
  for(unsigned int i = 2; i < x; ++i) {
    if((x % i) == 0) {
      divisor = i;
      break;
    }
  }
  return divisor;
}

WASM_EXPORT
int heavy_function(unsigned int x)
{
    unsigned int ret = 2;

    for(int i=0; i < x; i++)
    {
        ret = (ret * ret + 24) / (23);
    }

    return ret;
}

WASM_EXPORT
int call_is_prime(unsigned int x)
{
    return is_prime(x);
}

WASM_EXPORT
unsigned int euclid(unsigned int a, unsigned int b) {
    return b == 0 ? a : euclid(b, a % b);
}

WASM_EXPORT
int main() {
  //printf("hello, world!\n");

  /*if(!func_2(0))
      func_2(54);

  func_2(2);*/

  //return call_is_prime(73239581);
  //return call_is_prime(15485863);

  return capi_test();

  return 1;
}
