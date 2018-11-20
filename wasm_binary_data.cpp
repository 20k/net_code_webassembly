#include "wasm_binary_data.hpp"

#include <array>
#include <iostream>

struct parser
{
    std::vector<uint8_t> ptr;
    int32_t offset = 0;

    parser(const data& d)
    {
        ptr = d.ptr;
        offset = d.offset;
    }

    bool in_bounds(int32_t o)
    {
        return o < (int32_t)ptr.size();
    }

    void bounds_check(int32_t o)
    {
        if(!in_bounds(o))
            throw std::runtime_error("Failed boundary check " + std::to_string(o) + " real " + std::to_string(ptr.size()));
    }

    template<int N>
    void checked_fetch(const std::array<uint8_t, N>& arr)
    {
        bounds_check(offset + N);

        for(int32_t i=0; i < N; i++)
        {
            if(ptr[i + offset] != arr[i])
                throw std::runtime_error("No match in checked fetch, expected " + std::to_string(arr[i]) + " but got " + std::to_string(ptr[i + offset]));
        }

        offset += N;
    }
};

void wasm_binary_data::init(data d)
{
    parser p(d);

    p.checked_fetch<4>({0x00, 0x61, 0x73, 0x6D});
    p.checked_fetch<4>({0x01, 0x00, 0x00, 0x00});
}
