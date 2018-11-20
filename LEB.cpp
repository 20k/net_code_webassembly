#include "LEB.hpp"
#include <limits>
#include <assert.h>

void leb_tests()
{
    {
        data d = leb::unsigned_encode<uint64_t>(80);

        /*for(int i=0; i < d.size(); i++)
        {
            std::cout << std::to_string(d.ptr[i]) + "," << std::endl;
        }*/

        uint64_t original = leb::unsigned_decode<uint64_t>(d);

        //std::cout << "orig " << original << std::endl;

        assert(original == 80);
    }

    {
        data dtr = leb::unsigned_encode<uint64_t>(0);

        assert(leb::unsigned_decode<uint64_t>(dtr) == 0);
    }

    {
        int64_t check = std::numeric_limits<int64_t>::min();

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = std::numeric_limits<int64_t>::max();

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = -9934349;

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = 768590789078;

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = -1;

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        int64_t check = 1;

        data d = leb::signed_encode<int64_t>(check);

        assert(leb::signed_decode<int64_t>(d) == check);
    }

    {
        uint64_t check = std::numeric_limits<uint64_t>::min();

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = std::numeric_limits<uint64_t>::max();

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = -9934349;

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = 768590789078;

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = -1;

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }

    {
        uint64_t check = 1;

        data d = leb::unsigned_encode<uint64_t>(check);

        assert(leb::unsigned_decode<uint64_t>(d) == check);
    }
}

