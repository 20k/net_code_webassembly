#include "runtime_types.hpp"

struct frame
{
    types::vec<runtime::value> locals;
    ///moduleinst
};

struct activation
{
    types::s32 n{0};

    frame f;
};

struct label
{
    types::s32 n{0};
    ///?
    types::expr e;
};

struct stk
{
    std::variant<runtime::value, label, activation> s;
};

void runtime::store::invoke(const runtime::funcaddr& address, runtime::moduleinst& minst, const types::vec<runtime::value>& vals)
{
    uint32_t adr = (uint32_t)address;

    if(adr >= (uint32_t)funcs.size())
        throw std::runtime_error("Adr out of bounds");

    runtime::funcinst& finst = funcs[adr];

    types::functype ftype = finst.type;

    if(vals.size() != ftype.params.size())
        throw std::runtime_error("Argument mismatch");


}
