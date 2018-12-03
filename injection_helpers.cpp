#include "injection_helpers.hpp"
#include "runtime_types.hpp"

/*import1 env
import2 _ZSt15get_new_handlerv
import1 env
import2 __syscall1
import1 env
import2 __syscall3
import1 env
import2 __syscall5*/

///just immediately aborts
uint32_t _ZSt15get_new_handlerv()
{
    throw std::runtime_error("Attempt to get new handler!");
}

using stype = uint32_t;

template<typename... T>
uint32_t do_syscall(stype val, T... vals)
{
    throw std::runtime_error("Syscalls not supported");
}

auto syscall0 = do_syscall<>;
auto syscall1 = do_syscall<stype>;
auto syscall2 = do_syscall<stype, stype>;
auto syscall3 = do_syscall<stype, stype, stype>;
auto syscall4 = do_syscall<stype, stype, stype, stype>;
auto syscall5 = do_syscall<stype, stype, stype, stype, stype>;
auto syscall6 = do_syscall<stype, stype, stype, stype, stype, stype>;

std::map<std::string, std::map<std::string, runtime::externval>> get_env_helpers(runtime::store& s)
{
    runtime::externval new_handle;
    new_handle.val = runtime::allochostsimplefunction<_ZSt15get_new_handlerv>(s);

    runtime::externval s0;
    runtime::externval s1;
    runtime::externval s2;
    runtime::externval s3;
    runtime::externval s4;
    runtime::externval s5;
    runtime::externval s6;

    s0.val = runtime::allochostsimplefunction<syscall0>(s);
    s1.val = runtime::allochostsimplefunction<syscall1>(s);
    s2.val = runtime::allochostsimplefunction<syscall2>(s);
    s3.val = runtime::allochostsimplefunction<syscall3>(s);
    s4.val = runtime::allochostsimplefunction<syscall4>(s);
    s5.val = runtime::allochostsimplefunction<syscall5>(s);
    s6.val = runtime::allochostsimplefunction<syscall6>(s);

    std::map<std::string, std::map<std::string, runtime::externval>> vals;

    vals["env"]["_ZSt15get_new_handlerv"] = new_handle;

    vals["env"]["__syscall0"] = s0;
    vals["env"]["__syscall1"] = s1;
    vals["env"]["__syscall2"] = s2;
    vals["env"]["__syscall3"] = s3;
    vals["env"]["__syscall4"] = s4;
    vals["env"]["__syscall5"] = s5;
    vals["env"]["__syscall6"] = s6;

    return vals;
}

