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

template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T> && std::is_arithmetic_v<std::remove_reference_t<T>>>>
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
void lowest_get(serialisable& v, parser& p)
{
    v.handle_serialise(p, false);
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

inline
void lowest_get(types::typeidx& val, parser& p)
{
    val = leb::unsigned_decode<types::typeidx>(p);
}

inline
void lowest_get(types::funcidx& val, parser& p)
{
    val = leb::unsigned_decode<types::funcidx>(p);
}

inline
void lowest_get(types::tableidx& val, parser& p)
{
    val = leb::unsigned_decode<types::tableidx>(p);
}

inline
void lowest_get(types::memidx& val, parser& p)
{
    val = leb::unsigned_decode<types::memidx>(p);
}

inline
void lowest_get(types::globalidx& val, parser& p)
{
    val = leb::unsigned_decode<types::globalidx>(p);
}

inline
void lowest_get(types::localidx& val, parser& p)
{
    val = leb::unsigned_decode<types::localidx>(p);
}

inline
void lowest_get(types::labelidx& val, parser& p)
{
    val = leb::unsigned_decode<types::labelidx>(p);
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
void lowest_get(types::blocktype& val, parser& p)
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
        case 0x40:
            break;
        default:
            throw std::runtime_error("Didn't find a type in blocktype " + std::to_string(next));
            break;
    }
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
void lowest_get(types::elemtype& val, parser& p)
{
    uint8_t next = p.next();

    if(next != 0x70)
        throw std::runtime_error("Found wrong next elemtype " + std::to_string(next));

    val.which = next;
}

inline
void lowest_get(types::name& val, parser& p)
{
    lowest_get(val.dat, p);
}

inline
void lowest_get(types::limits& val, parser& p)
{
    uint8_t next = p.next();

    if(next != 0 && next != 1)
        throw std::runtime_error("Wrong limits prefix " + std::to_string(next));

    val.has_max_val = next;

    lowest_get(val.n, p);

    if(val.has_max_val)
    {
        lowest_get(val.m, p);
    }
}

inline
void lowest_get(types::tabletype& type, parser& p)
{
    lowest_get(type.et, p);
    lowest_get(type.lim, p);
}

inline
void lowest_get(types::mut& type, parser& p)
{
    uint8_t next = p.next();

    type.is_mut = next;
}

inline
void lowest_get(types::memtype& type, parser& p)
{
    lowest_get(type.lim, p);
}

inline
void lowest_get(types::globaltype& type, parser& p)
{
    lowest_get(type.type, p);
    lowest_get(type.m, p);
}

inline
void lowest_get(types::table& type, parser& p)
{
    lowest_get(type.type, p);
}

inline
void lowest_get(types::mem& type, parser& p)
{
    lowest_get(type.type, p);
}

inline
void lowest_get(types::memarg& type, parser& p)
{
    lowest_get(type.align, p);
    lowest_get(type.offset, p);
}

inline
void lowest_get(types::br_table_data& type, parser& p)
{
    lowest_get(type.labels, p);
    lowest_get(type.fin, p);
}

inline
void lowest_get(types::instr&, parser& p);

inline
void lowest_get(types::double_branch_data& type, parser& p)
{
    lowest_get(type.btype, p);

    while(p.peek() != 0x05 && p.peek() != 0x0B)
    {
        types::instr temp;
        lowest_get(temp, p);

        type.first.push_back(temp);
    }

    if(p.peek() == 0x05)
    {
        p.checked_fetch<1>({0x05});
        type.has_second_branch = true;

        while(p.peek() != 0x0B)
        {
            types::instr temp;
            lowest_get(temp, p);

            type.second.push_back(temp);
        }
    }

    p.checked_fetch<1>({0x0B});
}

inline
void lowest_get(types::single_branch_data& type, parser& p)
{
    lowest_get(type.btype, p);

    while(p.peek() != 0x0B)
    {
        types::instr temp;
        lowest_get(temp, p);

        type.first.push_back(temp);
    }

    p.checked_fetch<1>({0x0B});
}

template<typename T, typename... U>
inline
void lowest_get(std::variant<U...>& type, parser& p)
{
    T val;
    lowest_get(val, p);

    type = val;
}

///https://webassembly.github.io/spec/core/binary/instructions.html#binary-expr
inline
void lowest_get(types::instr& type, parser& p)
{
    uint8_t next = p.next();

    if(next == 0x0B)
        return;

    type.which = next;

    if(next == 0x41)
    {
        lowest_get<types::i32>(type.dat, p);
    }

    if(next == 0x42)
    {
        lowest_get<types::i64>(type.dat, p);
    }

    if(next == 0x43)
    {
        lowest_get<types::f32>(type.dat, p);
    }

    if(next == 0x44)
    {
        lowest_get<types::f64>(type.dat, p);
    }

    if(next >= 0x28 && next <= 0x3E)
    {
        lowest_get<types::memarg>(type.dat, p);
    }

    if(next >= 0x3F && next <= 0x40)
    {
        ///reserved for a future version of json
        uint8_t future_index_fun = 0;
        lowest_get(future_index_fun, p);
    }

    if(next >= 0x20 && next <= 0x22)
    {
        lowest_get<types::localidx>(type.dat, p);
    }

    if(next >= 0x23 && next <= 0x24)
    {
        lowest_get<types::globalidx>(type.dat, p);
    }

    if(next == 0x03 || next == 0x02)
    {
        lowest_get<types::single_branch_data>(type.dat, p);
    }

    if(next == 0x04)
    {
        lowest_get<types::double_branch_data>(type.dat, p);
    }

    if(next == 0x0C || next == 0x0D)
    {
        lowest_get<types::localidx>(type.dat, p);
    }

    if(next == 0x0E)
    {
        lowest_get<types::br_table_data>(type.dat, p);
    }

    if(next == 0x10)
    {
        lowest_get<types::funcidx>(type.dat, p);
    }

    if(next == 0x11)
    {
        lowest_get<types::typeidx>(type.dat, p);

        p.checked_fetch<1>({0x00});
    }
}

inline
void lowest_get(types::expr& type, parser& p)
{
    while(p.peek() != 0x0B)
    {
        types::instr temp;
        lowest_get(temp, p);

        type.i.push_back(temp);
    }

    p.checked_fetch<1>({0x0B});
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
