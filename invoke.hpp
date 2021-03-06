#include "types.hpp"
#include "runtime_types.hpp"
#include <iostream>

std::string instr_to_str(uint8_t which);

struct frame
{
    //types::vec<runtime::value> locals;
    types::svec<runtime::value, 8192> locals;
    ///hoo boy ok we're cracking out the pointers i guess
    ///this must be resolved for serialisation
    ///maybe use a module_idx and keep the modules in the stores
    runtime::moduleinst* inst = nullptr;
};

struct activation
{
    frame f;
    types::s32 return_arity{0};
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

struct full_stack
{
    ///todo tomorrow
    ///experiment with making these actual indexed stacks
    ///with a fixed max size and stuff
    types::vec<runtime::value> full;

    types::vec<uint32_t> stack_start_sizes{{0}};

    full_stack()
    {
        //full.reserve(1024);
    }

    void push_values(const runtime::value& val)
    {
        full.push_back(val);
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
    }

    void push_stack()
    {
        stack_start_sizes.push_back(full.size());
    }

    void pop_stack()
    {
        stack_start_sizes.pop_back();
    }

    ///this is unsafe because it doesn't set
    ///stasck_values.back() == 0
    ///however, in all uses of this function, its not an issue
    types::vec<runtime::value> pop_all_values_on_stack_unsafe()
    {
        types::vec<runtime::value> ret;

        size_t n = full.size();

        size_t start = stack_start_sizes.back();

        //if(start < 0)
        //    throw std::runtime_error("weird num error, not sure this is possible");

        for(size_t i=start; i < n; i++)
        {
            ret.push_back(full[i]);
        }

        full.resize(start);

        return ret;
    }

    ///for some reason, this is quite a bit slower
    ///than using the above. is probably cache related
    void pop_all_values_on_stack_unsafe_nocatch()
    {
        full.resize(stack_start_sizes.back());
    }

    types::vec<runtime::value> pop_num_vals(size_t num)
    {
        types::vec<runtime::value> ret;

        size_t n = full.size();

        size_t start = n - num;

        //if(start < 0)
        //    throw std::runtime_error("weird num error, not sure this is possible");

        for(size_t i=start; i < n; i++)
        {
            ret.push_back(full[i]);
        }

        //types::vec<runtime::value> ret(full.begin() + start, full.begin() + n);

        full.resize(start);

        return ret;
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

        return last;
    }

    void pop_2(runtime::value& v1, runtime::value& v2)
    {
        v2 = full.back();
        full.pop_back();
        v1 = full.back();
        full.pop_back();
    }

    std::optional<runtime::value> peek_back()
    {
        if(full.size() == 0)
            return std::nullopt;

        return full.back();
    }

    int value_stack_size()
    {
        return full.size();
    }

    auto current_stack_size()
    {
        return stack_start_sizes.back();
    }
};

types::vec<runtime::value> eval_with_frame(runtime::moduleinst& minst, runtime::store& s, const types::vec<types::instr>& exp);
