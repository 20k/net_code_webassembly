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

    void push_values(const runtime::value& val)
    {
        stk k;
        k.s = val;

        full.push_back(k);
    }

    types::vec<runtime::value> pop_num_vals(int num)
    {
        types::vec<runtime::value> r;

        for(int i = full.size() - 1; i > full.size() - 1 - num && i >= 0; i--)
        {
            stk& elem = full[i];

            if(!std::holds_alternative<runtime::value>(elem.s))
                throw std::runtime_error("Invalid arguments in get_num_vals");

            r.push_back(std::get<runtime::value>(elem.s));
        }

        if(r.size() != num)
            throw std::runtime_error("Could not get sufficient args");

        full.resize(full.size() - num);

        return r;
    }
};

void eval_expr(const types::expr& exp, full_stack& full);
