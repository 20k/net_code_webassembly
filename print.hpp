#ifndef PRINT_HPP_INCLUDED
#define PRINT_HPP_INCLUDED

inline
std::string binary_to_hex(const std::string& in, bool swap_endianness)
{
    std::string ret;

    const char* LUT = "0123456789ABCDEF";

    for(auto& i : in)
    {
        int lower_bits = ((int)i) & 0xF;
        int upper_bits = (((int)i) >> 4) & 0xF;

        if(swap_endianness)
        {
            std::swap(lower_bits, upper_bits);
        }

        ret += std::string(1, LUT[lower_bits]) + std::string(1, LUT[upper_bits]);
    }

    return ret;
}

template<typename T>
inline
void dump(const T& t)
{
    std::string str;
    str.resize(1);
    str[0] = t;

    std::cout << binary_to_hex(str, true);
}

#endif // PRINT_HPP_INCLUDED
