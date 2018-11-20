#ifndef LEB_HPP_INCLUDED
#define LEB_HPP_INCLUDED

#include "types.hpp"
#include <bitset>

namespace leb
{
    template<typename T, typename U>
    inline
    T signed_decode(U& in)
    {
        T result{0};
        uint32_t shift = 0;
        uint8_t byte = 0;

        //while(true)
        do
        {
            byte = in.next();

            T lowbits{0x7F & byte};

            result |= (lowbits << shift);

            shift += 7;
        }
        while((byte & 0x80) > 0);

        if(shift < sizeof(T) * 8 && (byte & 0x40) > 0)
        {
            T nzero{~0};
            nzero = nzero << shift;

            result |= nzero;
        }

        return result;
    }

    template<typename T, typename U>
    inline
    T unsigned_decode(U& in)
    {
        T result{0};
        uint64_t shift = 0;

        while(true)
        {
            uint8_t byte = in.next();

            T lowbits{(uint8_t)((uint8_t)0x7F & byte)};

            result |= (lowbits << shift);

            if((byte >> 7) == 0)
                break;

            shift += 7;
        }

        return result;
    }

    template<typename T, typename U>
    inline
    U signed_encode(T in)
    {
        U dtr;

        bool more = 1;
        //bool negative = (in < 0);

        while(more)
        {
            uint8_t byte = in & 0x7F;
            in >>= 7;

            if((in == 0 && ((byte & 0x40) == 0)) || (in == -1 && ((byte & 0x40) > 0)))
                more = 0;
            else
                byte |= 0x80;

            dtr.push_back(byte);
        }

        return dtr;
    }

    template<typename T, typename U>
    inline
    U unsigned_encode(T in)
    {
        U dtr;

        do
        {
            uint8_t byte = in & 0x7F;
            in >>= 7;

            if (in != 0)
                byte |= 0x80;

            dtr.push_back(byte);
        }
        while (in != 0);

        return dtr;
    }
}

void leb_tests();

#endif // LEB_HPP_INCLUDED
