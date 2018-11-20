#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <stdint.h>
#include <vector>

namespace types
{
    ///u32, u64, s32, s64, i8, i16, i32, i64.

    using i32 = uint32_t;
    using f32 = float;

    using i64 = uint64_t;
    using f64 = double;

    using u32 = i32;
    using u64 = i64;

    using s32 = int32_t;
    using s64 = int32_t;

    using i8 = uint8_t;
    using i16 = uint16_t;
}

struct data
{
    std::vector<uint8_t> ptr;
    uint32_t offset = 0;

    uint8_t next()
    {
        if(offset >= ptr.size())
            throw std::runtime_error("offset >= ptr.size()");

        uint8_t val = ptr[offset];

        offset++;

        return val;
    }

    int32_t size()
    {
        return ptr.size();
    }

    void push_back(uint8_t val)
    {
        ptr.push_back(val);
    }
};

#endif // TYPES_HPP_INCLUDED
