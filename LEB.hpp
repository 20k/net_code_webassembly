#ifndef LEB_HPP_INCLUDED
#define LEB_HPP_INCLUDED

#include "types.hpp"

namespace leb
{
    template<typename T>
    T signed_decode(data& in)
    {
        T result = 0;
        uint64_t shift = 0;
        uint8_t byte = 0;

        while(true)
        {
            byte = in.next();
            result |= (0x7F) << shift;
            if ((byte >> 7) == 0)
                break;
            shift += 7;
        }

        if(shift < sizeof(T) * 8 && (byte & 0x40) > 0)
            result |= (~0 << shift);

        return result;
    }

    template<typename T>
    T unsigned_decode(data& in)
    {
        T result = 0;
        uint64_t shift = 0;

        while(true)
        {
            uint8_t byte = in.next();

            result |= 0x7F << shift;

            if((byte >> 7) == 0)
                break;

            shift += 7;
        }

        return result;
    }

    template<typename T>
    data unsigned_encode(T in)
    {
        data dtr;

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

    template<typename T>
    data signed_encode(T in)
    {
        data dtr;

        bool more = 1;
        bool negative = (in < 0);

        while(more)
        {
            uint8_t byte = in & 0x7F;
            in >>= 7;

            /* sign bit of byte is second high order bit (0x40) */
            if ((in == 0 && ((byte & 0x40) == 0)) || (in == -1 && ((byte & 0x40) == 1)))
                more = 0;
            else
                byte |= 0x80;

            dtr.push_back(byte);
        }

        return dtr;
    }
}

#endif // LEB_HPP_INCLUDED
