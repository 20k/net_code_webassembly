#include "LEB.hpp"
#include <limits>
#include <assert.h>
#include <iostream>

void leb_tests()
{
    {
        data d = leb::unsigned_encode<uint64_t, data>(80);

        /*for(int i=0; i < d.size(); i++)
        {
            std::cout << std::to_string(d.ptr[i]) + "," << std::endl;
        }*/

        uint64_t original = leb::unsigned_decode<uint64_t>(d);

        //std::cout << "orig " << original << std::endl;

        assert(original == 80);
    }

    {
        data dtr = leb::unsigned_encode<uint64_t, data>(0);

        assert(leb::unsigned_decode<uint64_t>(dtr) == 0);
    }

    {
        int64_t check = std::numeric_limits<int64_t>::min();

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = std::numeric_limits<int64_t>::max();

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = -9934349;

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = 768590789078;

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = -1;

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = 1;

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        uint64_t check = std::numeric_limits<uint64_t>::min();

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = std::numeric_limits<uint64_t>::max();

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = -9934349;

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = 768590789078;

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = -1;

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = 1;

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = 2141192192;

        data d = leb::unsigned_encode<uint64_t, data>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        int64_t check = 2141192192;

        data d = leb::signed_encode<int64_t, data>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        data d;
        d.push_back(0x03);

        assert(leb::unsigned_decode<uint32_t>(d) == 3);
    }

    {
        data d;
        d.push_back(0x83);
        d.push_back(0x00);

        uint8_t val = leb::unsigned_decode<uint8_t>(d);

        assert(d.offset == 2);

        //std::cout << "VAL" << std::to_string(val) << std::endl;

        assert(val == 3);
    }

    {
        data d;
        d.push_back(0x7e);

        assert(leb::signed_decode<int16_t>(d) == -2);
    }
    {
        data d;
        d.push_back(0xfe);
        d.push_back(0x7f);

        assert(leb::signed_decode<int16_t>(d) == -2);
    }
    {
        data d;
        d.push_back(0xfe);
        d.push_back(0xff);
        d.push_back(0x7f);

        assert(leb::signed_decode<int16_t>(d) == -2);

        assert(d.offset == 3);
    }
}

