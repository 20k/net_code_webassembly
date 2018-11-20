#ifndef SERIALISABLE_HPP_INCLUDED
#define SERIALISABLE_HPP_INCLUDED

#include "types.hpp"

struct parser
{
    std::vector<uint8_t> ptr;
    int32_t offset = 0;

    parser(const data& d)
    {
        ptr = d.ptr;
        offset = d.offset;
    }

    uint8_t peek()
    {
        if(offset >= (int32_t)ptr.size())
            throw std::runtime_error("offset >= ptr.size()");

        return ptr[offset];
    }

    uint8_t next()
    {
        if(offset >= (int32_t)ptr.size())
            throw std::runtime_error("offset >= ptr.size()");

        uint8_t val = ptr[offset];

        offset++;

        return val;
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

    void advance(int32_t n)
    {
        bounds_check(offset + n);

        offset += n;
    }
};

struct serialisable
{
    virtual void handle_serialise(parser& p, bool ser){}
};

template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T> && std::is_standard_layout_v<std::remove_reference_t<T>>>>
inline
void lowest_get(T& v, parser& p)
{
    int32_t prev = p.offset;

    p.offset += sizeof(T);

    if(p.offset > (int32_t)p.ptr.size())
    {
        std::cout << "Error, invalid bytefetch low " << typeid(T).name() << std::endl;
        return;
    }

    memcpy(&v, &p.ptr[prev], sizeof(T));
}

inline
void lowest_get(types::i32& val, parser& p)
{
    val = leb::unsigned_decode<types::i32>(p);
}

inline
void lowest_get(types::u32& val, parser& p)
{
    val = leb::unsigned_decode<types::u32>(p);
}

inline
void lowest_get(types::s32& val, parser& p)
{
    assert(false);
}

inline
void lowest_get(types::i64& val, parser& p)
{
    val = leb::unsigned_decode<types::i64>(p);
}

inline
void lowest_get(types::u64& val, parser& p)
{
    val = leb::unsigned_decode<types::u64>(p);
}

inline
void lowest_get(types::s64& val, parser& p)
{
    assert(false);
}

inline
void lowest_get(types::f32& val, parser& p)
{
    lowest_get<float>(val.val, p);
}

inline
void lowest_get(types::f64& val, parser& p)
{
    lowest_get<double>(val.val, p);
}

template<typename T>
inline
void lowest_get(types::vec<T>& val, parser& p)
{
    types::u32 len;
    lowest_get(len, p);

    val.v.clear();
    val.v.reserve((uint32_t)len);

    for(uint32_t i=0; i < (uint32_t)len; i++)
    {
        T& back = val.v.emplace_back();

        lowest_get(back, p);
    }
}

inline
void lowest_get(types::valtype& val, parser& p)
{
    uint8_t next = p.next();

    val.which = next;

    switch(next)
    {
        case 0x7F:
            break;
        case 0x7E:
            break;
        case 0x7D:
            break;
        case 0x7C:
            break;
        default:
            throw std::runtime_error("Didn't find a type " + std::to_string(next));
            break;
    }

    /*switch(next)
    {
        case 0x7F:
            lowest_get(val.val.i_32, p);
            break;
        case 0x7E:
            lowest_get(val.val.i_64, p);
            break;
        case 0x7D:
            lowest_get(val.val.f_32, p);
            break;
        case 0x7C:
            lowest_get(val.val.f_64, p);
            break;
        default:
            throw std::runtime_error("Didn't find a type " + std::to_string(next));
            break;
    }*/
}

inline
void lowest_get(types::func& val, parser& p)
{
    uint8_t next = p.next();

    if(next != 0x60)
        throw std::runtime_error("Found wrong next " + std::to_string(next));

    lowest_get(val.params, p);
    lowest_get(val.results, p);
}

inline
void lowest_get(serialisable& v, parser& p)
{
    v.handle_serialise(p, false);
}

template<typename T>
void serialise(T& type, parser& p, bool ser)
{
    if(ser)
    {
        throw std::runtime_error("Writing not supported");
    }
    else
    {
        lowest_get(type, p);
    }
}


#endif // SERIALISABLE_HPP_INCLUDED
