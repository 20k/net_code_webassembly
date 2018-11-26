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
    //types::vec<int32_t> activation_offsets;
    //types::vec<activation> activation_stack;
    //types::vec<label> label_stack;

    types::vec<uint32_t> stack_start_sizes{{0}};

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

    void push_activation()
    {
        stack_start_sizes.push_back(full.size());

        /*std::cout <<" in a " << a.f.locals.size() << std::endl;
        std::cout << "dbga " << get_current().f.locals.size() << std::endl;*/
    }

    void push_label()
    {
        stack_start_sizes.push_back(full.size());
    }

    ///this is unsafe because it doesn't set
    ///stasck_values.back() == 0
    ///however, in all uses of this function, its not an issue
    types::vec<runtime::value> pop_all_values_on_stack_unsafe()
    {
        types::vec<runtime::value> ret;

        int32_t to_pop = full.size() - stack_start_sizes.back();

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

        return ret;
    }

    ///for some reason, this is quite a bit slower
    ///than using the above. is probably cache related
    void pop_all_values_on_stack_unsafe_nocatch()
    {
        full.resize(stack_start_sizes.back());
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

        //types::vec<runtime::value> ret(full.begin() + start, full.begin() + n);

        full.resize(n - num);

        return ret;
    }

    void pop_back_activation()
    {
        stack_start_sizes.pop_back();
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

    void pop_back_label()
    {
        stack_start_sizes.pop_back();
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

    int current_stack_size()
    {
        return stack_start_sizes.back();
    }
};

types::vec<runtime::value> eval_with_frame(runtime::moduleinst& minst, runtime::store& s, const types::vec<types::instr>& exp);

struct context;
struct label;
struct full_stack;

struct info_stack
{
    int pc = 0;

    ///type 1 = label
    ///type 2 = activation
    int type = 0;
    int offset = 0;
    bool should_loop = false;
    bool has_delayed_values_push = false;

    context& ctx;
    full_stack& full;

    const types::vec<types::instr>& in;

    info_stack(context& _ctx, runtime::store& s, full_stack& _full, const runtime::funcaddr& address, runtime::moduleinst& minst);
    info_stack(context& _ctx, const label& l, const types::vec<types::instr>& exp, full_stack& _full);

    const types::vec<types::instr>& start_label(context& ctx, const label& l, const types::vec<types::instr>& exp, full_stack& full);
    const types::vec<types::instr>& start_function(runtime::store& s, full_stack& full, const runtime::funcaddr& address, runtime::moduleinst& minst);

    bool loop();

    void end_label(context& ctx, full_stack& full);
    void end_function(context& ctx, full_stack& full);
    types::vec<runtime::value> end_function_final(context& ctx, full_stack& full);

    void destroy();
};
