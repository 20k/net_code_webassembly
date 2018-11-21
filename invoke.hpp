#include "types.hpp"
#include "runtime_types.hpp"

struct frame
{
    types::vec<runtime::value> locals;
    ///moduleinst
};

struct activation
{
    types::s32 return_arity{0};

    frame f;
};

struct label
{
    types::s32 argument_arity{0};
    ///?
    types::expr e;
};

struct stk
{
    std::variant<runtime::value, label, activation> s;
};

struct full_stack
{
    types::vec<stk> full;

    void push_value(const runtime::value& val)
    {
        stk k;
        k.s = val;

        full.push_back(k);
    }
};

void eval_expr(const types::expr& exp, full_stack& full);
