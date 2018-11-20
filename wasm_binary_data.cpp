#include "wasm_binary_data.hpp"

#include <array>
#include <iostream>
#include "types.hpp"
#include "LEB.hpp"
#include <assert.h>
#include "serialisable.hpp"

struct section : serialisable
{
    uint8_t id = 0;
    types::u32 len{0};

    virtual void handle_serialise(parser& p, bool ser) override
    {
        serialise(id, p, ser);
        serialise(len, p, ser);
    }
};

namespace sections
{
    struct custom : section
    {
        virtual void handle_serialise(parser& p, bool ser) override
        {
            section::handle_serialise(p, ser);

            p.advance((uint32_t)len);
        }
    };

    struct type : section
    {
        virtual void handle_serialise(parser& p, bool ser) override
        {
            section::handle_serialise(p, ser);

        }
    };
}

void wasm_binary_data::init(data d)
{
    parser p(d);

    p.checked_fetch<4>({0x00, 0x61, 0x73, 0x6D});
    p.checked_fetch<4>({0x01, 0x00, 0x00, 0x00});
}
