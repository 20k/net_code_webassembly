#include "JIT.hpp"
#include <cstdint>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <Memoryapi.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif // _WIN32

size_t get_page_size()
{
    #ifdef _WIN32
    SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwPageSize;
	#else
    return sysconf(_SC_PAGE_SIZE);
	#endif
}

size_t get_estimated_mem_size(size_t code_size);

#ifdef _WIN32
void* get_executable_memory_map(size_t len)
{
    return VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

void unmap(void* ptr, size_t len)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}
#else
void* get_executable_memory_map(size_t len)
{
    return mmap(NULL,        // address
           4096,             // size
           PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS,
           -1,               // fd (not used here)
           0);               // offset (not used here)
}

void* unmap(void* ptr, size_t len)
{
    munmap(ptr, len);
}
#endif

//https://solarianprogrammer.com/2018/01/10/writing-minimal-x86-64-jit-compiler-cpp/

size_t get_estimated_mem_size(size_t code_size)
{
    size_t page_size_multiple = get_page_size();     // Get the machine page size
    size_t factor = 1;
    size_t required_memory_size = code_size;

    for(;;)
    {
        required_memory_size = factor * page_size_multiple;

        if(code_size <= required_memory_size)
            break;

        factor++;
    }

    return required_memory_size;
}

using fn = long(*)(long);

fn test_function(char* memory)
{
    int i = 0;


    // mov %rdi, %rax
    memory[i++] = 0x48;           // REX.W prefix
    memory[i++] = 0x8b;           // MOV opcode, register/register
    #ifdef _WIN32
    memory[i++] = 0xC1;           // MOD/RM byte for %rcx -> %rax
    #else
    memory[i++] = 0xc7;           // MOD/RM byte for %rdi -> %rax
    #endif // _WIN32

    // ret
    memory[i++] = 0xc3;           // RET opcode

    return (fn)memory;
}

void test_jit()
{
    char* my_ptr = (char*)get_executable_memory_map(4096);

    fn my_fn = test_function(my_ptr);

    for(int i=0; i < 10; i++)
    {
        printf("f(%d) = %ld\n", i, (*my_fn)(i));
    }

    unmap(my_ptr, 4096);
}
