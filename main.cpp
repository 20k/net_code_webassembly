#include <iostream>
#include "LEB.hpp"
#include <assert.h>
#include <limits>

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

        std::cout << "check " << check << std::endl;

        data d = leb::signed_encode<int64_t>(check);

        for(int i=0; i < d.size(); i++)
        {
            std::cout << std::to_string(d.ptr[i]) + "," << std::endl;
        }

        int64_t decoded = leb::signed_decode<int64_t>(d);

        std::cout << "decoded " << decoded << std::endl;

        assert(decoded == check);
    }
}

int main()
{
    leb_tests();

    return 0;
}
