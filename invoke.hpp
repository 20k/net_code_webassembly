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
    //std::variant<runtime::value, label> s;

    runtime::value s;

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
    types::vec<label> label_stack;

    types::vec<uint32_t> stack_values{{0}};

    void push_values(const runtime::value& val)
    {
        stk k;
        k.s = val;

        full.push_back(k);

        stack_values.back() += 1;
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

        stack_values.push_back(0);

        /*std::cout <<" in a " << a.f.locals.size() << std::endl;
        std::cout << "dbga " << get_current().f.locals.size() << std::endl;*/
    }

    void push_label(const label& l)
    {
        label_stack.push_back(l);

        stack_values.push_back(0);
    }

    types::vec<runtime::value> pop_all_values_on_stack()
    {
        /*int num = 0;
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

        return ret;*/

        types::vec<runtime::value> ret;

        int32_t to_pop = stack_values.back();

        for(int i = full.size() - 1; i >= 0 && i > full.size() - 1 - to_pop; i--)
        {
            ret.push_back(full[i].s);
        }

        if(ret.size() != to_pop)
            throw std::runtime_error("weird num error, not sure this is possible");

        full.resize(full.size() - to_pop);

        stack_values.back() = 0;

        return ret;
    }

    types::vec<runtime::value> pop_num_vals(int num)
    {
        /*types::vec<runtime::value> r;

        for(int i = full.size() - 1; i > full.size() - 1 - num && i >= 0; i--)
        {
            stk& elem = full[i];

            if(!std::holds_alternative<runtime::value>(elem.s))
                throw std::runtime_error("Invalid arguments in get_num_vals");

            r.push_back(std::get<runtime::value>(elem.s));
        }

        if(r.size() != num)
            throw std::runtime_error("Could not get sufficient args");

        full.resize(full.size() - num);*/

        types::vec<runtime::value> ret;

        for(int i = full.size() - 1; i >= 0 && i > full.size() - 1 - num; i--)
        {
            ret.push_back(full[i].s);
        }

        if(num != ret.size())
            throw std::runtime_error("Could not get sufficient args");

        full.resize(full.size() - num);

        stack_values.back() -= num;

        return ret;
    }

    void pop_back_activation()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("No elements on stack (pop_back_frame)");

        activation_stack.pop_back();

        stack_values.pop_back();
    }

    activation& get_current()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("rip activation stack");

        return activation_stack.back();
    }

    label& get_current_label()
    {
        /*for(int i=full.size() - 1; i >= 0; i--)
        {
            if(std::holds_alternative<label>(full[i].s))
                return std::get<label>(full[i].s);
        }

        throw std::runtime_error("No current label");*/

        if(label_stack.size() == 0)
            throw std::runtime_error("No current label");

        return label_stack.back();
    }

    label& get_label_of_offset(int offset)
    {
        offset++;

        /*int coffset = 0;

        for(int i=full.size() - 1; i >= 0; i--)
        {
            if(std::holds_alternative<label>(full[i].s))
            {
                coffset++;

                if(coffset == offset)
                    return std::get<label>(full[i].s);
            }
        }*/

        int coffset = 0;

        for(int i=label_stack.size() - 1; i >= 0; i--)
        {
            coffset++;

            if(coffset == offset)
                return label_stack[i];
        }

        throw std::runtime_error("Could not get label of offset " + std::to_string(offset));
    }

    void ensure_activation()
    {
        if(activation_stack.size() == 0)
            throw std::runtime_error("No stack");
    }

    void ensure_label()
    {
        /*if(full.size() == 0 || !std::holds_alternative<label>(full.back().s))
            throw std::runtime_error("No label in eval with label");*/

        if(label_stack.size() == 0)
            throw std::runtime_error("No label in eval with label");
    }

    runtime::value pop_back()
    {
        if(full.size() == 0)
            throw std::runtime_error("0 stack");

        auto last = full.back();

        /*if(!std::holds_alternative<runtime::value>(last.s))
            throw std::runtime_error("pop back on wrong type");*/

        full.pop_back();
        stack_values.back() -= 1;

        return last.s;
    }

    label pop_back_label()
    {
        if(label_stack.size() == 0)
            throw std::runtime_error("0 label stack");

        auto last = label_stack.back();

        label_stack.pop_back();
        stack_values.pop_back();

        return last;
    }

    std::optional<runtime::value> peek_back()
    {
        if(full.size() == 0)
            return std::nullopt;

        /*if(!std::holds_alternative<runtime::value>(full.back().s))
            return std::nullopt;*/

        return full.back().s;
        //return std::get<runtime::value>(full.back().s);
    }

    int num_labels()
    {
        /*int num = 0;

        for(stk& s : full)
        {
            if(std::holds_alternative<label>(s.s))
                num++;
        }

        return num;*/

        return label_stack.size();
    }
};

runtime::value eval_implicit(runtime::store& s, const types::vec<types::instr>& exp);
