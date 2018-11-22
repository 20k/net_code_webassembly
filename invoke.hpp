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
    types::blocktype btype;

    ///0 = none
    ///1 = end of block
    ///2 = start of block
    ///3 = end of if instruction
    uint8_t continuation = 0;
};

struct stk
{
    std::variant<runtime::value, label> s;

    /*void operator=(const runtime::value& val)
    {
        s = val;
    }

    stk(const runtime::value& val)
    {
        s = val;
    }

    stk()
    {

    }*/
};

struct full_stack
{
    types::vec<stk> full;
    //types::vec<int32_t> activation_offsets;
    types::vec<activation> activation_stack;

    void push_values(const runtime::value& val)
    {
        stk k;
        k.s = val;

        full.push_back(k);
    }

    void push_all_values(const types::vec<runtime::value>& val)
    {
        for(const auto& i : val)
        {
            push_values(i);
        }

        //full.insert(full.end(), val.begin(), val.end());
    }

    void push_activation(const activation& a)
    {
        activation_stack.push_back(a);

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

    void pop_back_activation()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("No elements on stack (pop_back_frame)");

        activation_stack.pop_back();
    }

    activation& get_current()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("rip activation stack");

        return activation_stack.back();
    }

    label& get_current_label()
    {
        for(int i=full.size() - 1; i >= 0; i--)
        {
            if(std::holds_alternative<label>(full[i].s))
                return std::get<label>(full[i].s);
        }

        throw std::runtime_error("No current label");
    }

    label& get_label_of_offset(int offset)
    {
        offset++;

        int coffset = 0;

        for(int i=full.size() - 1; i >= 0; i--)
        {
            if(std::holds_alternative<label>(full[i].s))
            {
                coffset++;

                if(coffset == offset)
                    return std::get<label>(full[i].s);
            }
        }

        throw std::runtime_error("Could not get label of offset " + std::to_string(offset));
    }

    void ensure_activation()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("No stack");
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

    label pop_back_label()
    {
        if(full.size() == 0)
            throw std::runtime_error("0 stack");

        auto last = full.back();

        if(!std::holds_alternative<label>(last.s))
            throw std::runtime_error("pop back on wrong type");

        full.pop_back();

        return std::get<label>(last.s);
    }

    std::optional<runtime::value> peek_back()
    {
        if(full.size() == 0)
            return std::nullopt;

        if(!std::holds_alternative<runtime::value>(full.back().s))
            return std::nullopt;

        return std::get<runtime::value>(full.back().s);
    }

    int num_labels()
    {
        int num = 0;

        for(stk& s : full)
        {
            if(std::holds_alternative<label>(s.s))
                num++;
        }

        return num;
    }
};

runtime::value eval_implicit(runtime::store& s, const types::vec<types::instr>& exp);
