#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gameapi.hpp"

#define WASM_EXPORT __attribute__ ((visibility ("default"), used)) extern "C"

//#define WASM_EXPORT __attribute__ ((used))

extern "C" int needs_import(const char* x);

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
int main() {
  //printf("hello, world!\n");

  if(!func_2(0))
      func_2(54);

  func_2(2);
  return 1;
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
