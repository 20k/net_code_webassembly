#include "injection_helpers.hpp"
#include "runtime_types.hpp"
#include <iostream>

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

constexpr auto syscall0 = do_syscall<>;
constexpr auto syscall1 = do_syscall<stype>;
constexpr auto syscall2 = do_syscall<stype, stype>;
constexpr auto syscall3 = do_syscall<stype, stype, stype>;
constexpr auto syscall4 = do_syscall<stype, stype, stype, stype>;
constexpr auto syscall5 = do_syscall<stype, stype, stype, stype, stype>;
constexpr auto syscall6 = do_syscall<stype, stype, stype, stype, stype, stype>;

using game_api_t = uint32_t;

struct c_str
{
    std::string storage;

    c_str(uint8_t* iptr, runtime::store* s)
    {
        int offset_from_start = iptr - s->get_memory_base_ptr();
        int mem_size = s->get_memory_base_size();

        uint8_t* base_ptr = s->get_memory_base_ptr();

        int slen = 0;

        for(int i=offset_from_start; i < mem_size && base_ptr[i] != '\0'; i++)
        {
            slen++;
        }

        if(slen == 0)
            return;

        storage = std::string((char*)iptr, slen);
    }

    std::string to_str()
    {
        return storage;
    }
};

///so
///now we need a context pointer in the webassembly instance that we can use to store intermediate data?
void serialise_object_begin(runtime::store* s, uint32_t gapi, char* key_in)
{
    auto it = s->interop_context.last_built.find(gapi);

    if(it == s->interop_context.last_built.end() || it->second.size() == 0)
        throw std::runtime_error("Bad object in subkey");

    std::shared_ptr<interop_element> last = it->second.back();

    std::shared_ptr<interop_element> next_ptr = std::make_shared<interop_element>();

    c_str key((uint8_t*)key_in, s);

    //std::cout << "key: " << key.to_str() << std::endl;

    //last->data[key->to_str()] = next_ptr;

    if(key.to_str() == "")
    {
        throw std::runtime_error("No key object");

        //last->data = std::map<std::string, std::shared_ptr<interop_element>>{{}
    }
    else
    {
        if(!std::holds_alternative<interop_element::object>(last->data))
        {
            last->data = interop_element::object{{key.to_str(), next_ptr}};
        }
        else
        {
            std::get<interop_element::object>(last->data)[key.to_str()] = next_ptr;
        }
    }

    it->second.push_back(next_ptr);
}

void serialise_object_end(runtime::store* s, uint32_t gapi, char* key_in)
{
    auto it = s->interop_context.last_built.find(gapi);

    if(it == s->interop_context.last_built.end() || it->second.size() == 0)
        throw std::runtime_error("Bad object in subkey");

    it->second.pop_back();
}

void serialise_object_begin_base(runtime::store* s, uint32_t gapi)
{
    ///WARNING DO SAFETY CHECK
    ///assert that last_built is empty
    ///assert that gapi doesn't exist already?

    s->interop_context.elems[gapi] = std::make_shared<interop_element>();

    s->interop_context.last_built[gapi].push_back(s->interop_context.elems[gapi]);
}

void serialise_object_end_base(runtime::store* s, uint32_t gapi)
{
    auto it = s->interop_context.last_built.find(gapi);

    if(it->second.size() != 1)
    {
        throw std::runtime_error("Mismatch object build");
    }

    if(it != s->interop_context.last_built.end())
    {
        s->interop_context.last_built.erase(it);
    }
}

void serialise_basic_u32(runtime::store* s, uint32_t gapi, uint32_t* u, char* key_in, bool ser)
{

}

void serialise_basic_u64(runtime::store* s, uint32_t gapi, uint64_t* u, char* key_in, bool ser)
{

}

void serialise_basic_float(runtime::store* s, uint32_t gapi, float* u, char* key_in, bool ser)
{

}

void serialise_basic_double(runtime::store* s, uint32_t gapi, double* u, char* key_in, bool ser)
{

}

void serialise_basic_string(runtime::store* s, uint32_t gapi, c_str* u, char* key_in, bool ser)
{

}

///will need a serialise function function so we can pass functions across the boundary
///the c api for a function will take a vector of parameters and an optional return type
///then will have to template magic jazz hands back to c++

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

    runtime::externval serialise_begin;
    runtime::externval serialise_end;

    runtime::externval serialise_begin_base;
    runtime::externval serialise_end_base;

    runtime::externval serialise_u32;
    runtime::externval serialise_u64;
    runtime::externval serialise_float;
    runtime::externval serialise_double;
    runtime::externval serialise_string;

    serialise_begin.val = runtime::allochostsimplefunction<serialise_object_begin>(s);
    serialise_end.val = runtime::allochostsimplefunction<serialise_object_end>(s);

    serialise_begin_base.val = runtime::allochostsimplefunction<serialise_object_begin_base>(s);
    serialise_end_base.val = runtime::allochostsimplefunction<serialise_object_end_base>(s);

    serialise_u32.val = runtime::allochostsimplefunction<serialise_basic_u32>(s);
    serialise_u64.val = runtime::allochostsimplefunction<serialise_basic_u64>(s);
    serialise_float.val = runtime::allochostsimplefunction<serialise_basic_float>(s);
    serialise_double.val = runtime::allochostsimplefunction<serialise_basic_double>(s);
    serialise_string.val = runtime::allochostsimplefunction<serialise_basic_string>(s);

    std::map<std::string, std::map<std::string, runtime::externval>> vals;

    vals["env"]["_ZSt15get_new_handlerv"] = new_handle;

    vals["env"]["__syscall0"] = s0;
    vals["env"]["__syscall1"] = s1;
    vals["env"]["__syscall2"] = s2;
    vals["env"]["__syscall3"] = s3;
    vals["env"]["__syscall4"] = s4;
    vals["env"]["__syscall5"] = s5;
    vals["env"]["__syscall6"] = s6;

    vals["env"]["serialise_object_begin"] = serialise_begin;
    vals["env"]["serialise_object_end"] = serialise_end;

    vals["env"]["serialise_object_begin_base"] = serialise_begin_base;
    vals["env"]["serialise_object_end_base"] = serialise_end_base;

    vals["env"]["serialise_basic_u32"] = serialise_u32;
    vals["env"]["serialise_basic_u64"] = serialise_u64;
    vals["env"]["serialise_basic_float"] = serialise_float;
    vals["env"]["serialise_basic_double"] = serialise_double;
    vals["env"]["serialise_basic_string"] = serialise_string;

    return vals;
}

