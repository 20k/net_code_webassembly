#include "types.hpp"
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

struct full_stack
{
    types::vec<stk> full;
};

void eval_expr(const types::expr& exp, full_stack& full);
