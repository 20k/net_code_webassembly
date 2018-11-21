#include "types.hpp"
#include "runtime_types.hpp"
#include <iostream>

struct frame
{
    types::vec<runtime::value> locals;
    ///hoo boy ok we're cracking out the pointers i guess
    ///this must be resolved for serialisation
    ///maybe use a module_idx and keep the modules in the stores
    runtime::moduleinst* inst = nullptr;
};

struct activation
{
    types::s32 return_arity{0};

    frame f;
};

struct label
{
    /*types::s32 argument_arity{0};
    ///?
    types::expr e;*/

    types::single_branch_data dat;
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

    void push_activation(const activation& a)
    {
        stk k;
        k.s = a;

        full.push_back(k);

        /*std::cout <<" in a " << a.f.locals.size() << std::endl;
        std::cout << "dbga " << get_current().f.locals.size() << std::endl;*/
    }

    void push_label(const label& l)
    {
        stk k;
        k.s = l;

        full.push_back(k);
    }

    types::vec<runtime::value> pop_all_values_on_stack()
    {
        int num = 0;
        types::vec<runtime::value> ret;

        for(int i = full.size() - 1; i >= 0; i--)
        {
            stk& elem = full[i];

            if(!std::holds_alternative<runtime::value>(elem.s))
                break;

            num++;
            ret.push_back(std::get<runtime::value>(elem.s));
        }

        if(num < 0 || num >= full.size())
            throw std::runtime_error("weird num error, not sure this is possible");

        full.resize(full.size() - num);

        return ret;
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

    activation& get_current()
    {
        for(int i=full.size() - 1; i >= 0; i--)
        {
            if(std::holds_alternative<activation>(full[i].s))
                return std::get<activation>(full[i].s);
        }

        throw std::runtime_error("No current activation");
    }

    void ensure_activation()
    {
        if(full.size() == 0)
            throw std::runtime_error("No stack");

        if(std::holds_alternative<activation>(full.back().s))
            return;

        throw std::runtime_error("No activation on stack");
    }

    runtime::value pop_back()
    {
        if(full.size() == 0)
            throw std::runtime_error("0 stack");

        auto last = full.back();

        if(!std::holds_alternative<runtime::value>(last.s))
            throw std::runtime_error("pop back on wrong type");

        full.pop_back();

        return std::get<runtime::value>(last.s);
    }
};

void eval_expr(runtime::store& s, const types::expr& exp, full_stack& full);
