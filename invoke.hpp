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
    types::vec<runtime::value> full;
    //types::vec<int32_t> activation_offsets;
    types::vec<activation> activation_stack;
    types::vec<label> label_stack;

    types::vec<uint32_t> stack_values{{0}};

    void push_values(const runtime::value& val)
    {
        full.push_back(val);

        stack_values.back() += 1;
    }

    /*template<typename T>
    void push_values(const T& t)
    {
        full.emplace_back(t);

        stack_values.back() += 1;
    }*/

    void push_all_values(const types::vec<runtime::value>& val)
    {
        full.insert(full.end(), val.begin(), val.end());
        stack_values.back() += val.size();
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

    ///this is unsafe because it doesn't set
    ///stasck_values.back() == 0
    ///however, in all uses of this function, its not an issue
    types::vec<runtime::value> pop_all_values_on_stack_unsafe()
    {
        types::vec<runtime::value> ret;

        int32_t to_pop = stack_values.back();

        int32_t n = full.size();

        int32_t start = full.size() - to_pop;

        if(start < 0)
            throw std::runtime_error("weird num error, not sure this is possible");

        for(int i=start; i < n; i++)
        {
            ret.push_back(full[i]);
        }

        /*if(ret.size() != to_pop)
            throw std::runtime_error("weird num error, not sure this is possible");*/

        full.resize(full.size() - to_pop);

        //stack_values.back() = 0;

        return ret;
    }

    ///for some reason, this is quite a bit slower
    ///than using the above. is probably cache related
    void pop_all_values_on_stack_unsafe_nocatch()
    {
        int32_t to_pop = stack_values.back();

        int32_t start = full.size() - to_pop;

        //if(start < 0)
        //    throw std::runtime_error("Bad start in pop all");

        full.resize(start);
    }

    types::vec<runtime::value> pop_num_vals(int num)
    {
        types::vec<runtime::value> ret;

        int32_t n = full.size();

        int32_t start = n - num;

        //if(start < 0)
        //    throw std::runtime_error("weird num error, not sure this is possible");

        for(int i=start; i < n; i++)
        {
            ret.push_back(full[i]);
        }

        full.resize(n - num);

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
        if(label_stack.size() == 0)
            throw std::runtime_error("No current label");

        return label_stack.back();
    }

    label& get_label_of_offset(int offset)
    {
        offset++;

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
        if(label_stack.size() == 0)
            throw std::runtime_error("No label in eval with label");
    }

    runtime::value pop_back()
    {
        ///adds about 10ms
        #ifdef SAFER
        if(full.size() == 0)
            throw std::runtime_error("0 stack");
        #endif // SAFER

        auto last = full.back();

        full.pop_back();
        stack_values.back() -= 1;

        return last;
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

        return full.back();
    }

    int num_labels()
    {
        return label_stack.size();
    }
};

runtime::value eval_implicit(runtime::store& s, const types::vec<types::instr>& exp);
