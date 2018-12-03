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

    c_str(uint8_t* iptr, int64_t string_length, runtime::store* s)
    {
        int offset_from_start = iptr - s->get_memory_base_ptr();
        int mem_size = s->get_memory_base_size();

        int slen = 0;

        for(int i=offset_from_start; i < mem_size && slen < string_length; i++)
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

template<typename T>
void generic_serialise(runtime::store* s, uint32_t gapi, T* type, char* key_in, bool ser)
{
    c_str key((uint8_t*)key_in, s);

    if(ser)
    {
        std::shared_ptr<interop_element> ielem = s->interop_context.get_back(s, gapi);

        ielem->update_object_element(key.to_str(), *type);
    }
    else
    {
        //try
        {
            std::shared_ptr<interop_element> last = s->interop_context.get_back(s, gapi);

            if(!std::holds_alternative<interop_element::object>(last->data))
                throw std::runtime_error("Expected object in deserialise");

            ///get element by key
            auto elem_it = std::get<interop_element::object>(last->data).find(key.to_str());

            if(elem_it == std::get<interop_element::object>(last->data).end())
                throw std::runtime_error("Did not find object with key " + key.to_str());

            std::shared_ptr<interop_element> ielem = elem_it->second;

            if(std::holds_alternative<nlohmann::json>(ielem->data))
            {
                *type = (T)std::get<nlohmann::json>(ielem->data);
            }
            else if(std::holds_alternative<interop_element::func_ptr>(ielem->data))
            {
                throw std::runtime_error("func_ptr not implemented yet");
            }
            else if(std::holds_alternative<interop_element::object>(ielem->data))
            {
                ///???
                throw std::runtime_error("Did not expect object");
            }
            else
            {
                *type = T();
            }
        }
        /*catch(...)
        {
            *type = T();
        }*/
    }
}

///so
///now we need a context pointer in the webassembly instance that we can use to store intermediate data?
void serialise_object_begin(runtime::store* s, uint32_t gapi, char* key_in, bool ser)
{
    auto it = s->interop_context.last_built.find(gapi);

    if(it == s->interop_context.last_built.end() || it->second.size() == 0)
        throw std::runtime_error("Bad object in subkey");

    std::shared_ptr<interop_element> last = it->second.back();

    c_str key((uint8_t*)key_in, s);

    if(ser)
    {
        std::shared_ptr<interop_element> next_ptr = std::make_shared<interop_element>();

        if(key.to_str() == "")
        {
            throw std::runtime_error("No key object");
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
    else
    {
        ///so the current object is last, and we're expecting another object at last.key
        if(!std::holds_alternative<interop_element::object>(last->data))
            throw std::runtime_error("Expected object in deserialisation");

        auto elem_it = std::get<interop_element::object>(last->data).find(key.to_str());

        if(elem_it == std::get<interop_element::object>(last->data).end())
            throw std::runtime_error("Did not find object with key " + key.to_str());

        std::shared_ptr<interop_element> next = elem_it->second;

        it->second.push_back(next);
    }
}

void serialise_object_end(runtime::store* s, uint32_t gapi, char* key_in, bool ser)
{
    ///works for true/false ser

    auto it = s->interop_context.last_built.find(gapi);

    if(it == s->interop_context.last_built.end() || it->second.size() == 0)
        throw std::runtime_error("Bad object in subkey");

    it->second.pop_back();
}

void serialise_object_begin_base(runtime::store* s, uint32_t gapi, bool ser)
{
    ///WARNING DO SAFETY CHECK
    ///assert that last_built is empty
    ///assert that gapi doesn't exist already?
    if(ser)
        s->interop_context.elems[gapi] = std::make_shared<interop_element>();

    if(s->interop_context.elems.find(gapi) == s->interop_context.elems.end())
        throw std::runtime_error("No such game_api_t");

    s->interop_context.last_built[gapi].push_back(s->interop_context.elems[gapi]);
}

void serialise_object_end_base(runtime::store* s, uint32_t gapi, bool ser)
{
    ///handles true/false ser

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

constexpr void (*serialise_basic_u32)(runtime::store* s, uint32_t gapi, uint32_t* u, char* key_in, bool ser) = generic_serialise<uint32_t>;
constexpr void (*serialise_basic_u64)(runtime::store* s, uint32_t gapi, uint64_t* u, char* key_in, bool ser) = generic_serialise<uint64_t>;
constexpr void (*serialise_basic_float)(runtime::store* s, uint32_t gapi, float* u, char* key_in, bool ser) = generic_serialise<float>;
constexpr void (*serialise_basic_double)(runtime::store* s, uint32_t gapi, double* u, char* key_in, bool ser) = generic_serialise<double>;

void serialise_basic_string_length(runtime::store* s, uint32_t gapi, uint32_t* len, char* key_in)
{
    std::shared_ptr<interop_element> last = s->interop_context.get_back(s, gapi);

    if(!std::holds_alternative<interop_element::object>(last->data))
        throw std::runtime_error("Not object");

    c_str key((uint8_t*)key_in, s);

    auto elem_it = std::get<interop_element::object>(last->data).find(key.to_str());

    if(elem_it == std::get<interop_element::object>(last->data).end())
        throw std::runtime_error("Did not find object with key " + key.to_str());

    std::shared_ptr<interop_element> next = elem_it->second;

    if(!std::holds_alternative<nlohmann::json>(next->data))
    {
        throw std::runtime_error("Expected data type in key");
    }

    std::string str = (std::string)std::get<nlohmann::json>(next->data);

    *len = str.length();
}

void serialise_basic_string(runtime::store* s, uint32_t gapi, char* u, uint32_t l, char* key_in, bool ser)
{
    c_str key((uint8_t*)key_in, s);
    c_str val((uint8_t*)u, l, s);

    std::shared_ptr<interop_element> ielem = s->interop_context.get_back(s, gapi);

    if(ser)
    {
        ielem->update_object_element(key.to_str(), val.to_str());
    }
    else
    {
        ///get key
        ///assume that the char* pointer we're seeing is a pointer that is adequately resized
        ///make sure we don't write out of bounds obviously

        std::shared_ptr<interop_element> last = s->interop_context.get_back(s, gapi);

        if(!std::holds_alternative<interop_element::object>(last->data))
            throw std::runtime_error("Not object");

        auto elem_it = std::get<interop_element::object>(last->data).find(key.to_str());

        if(elem_it == std::get<interop_element::object>(last->data).end())
            throw std::runtime_error("Did not find object with key " + key.to_str());

        std::shared_ptr<interop_element> next = elem_it->second;

        if(!std::holds_alternative<nlohmann::json>(next->data))
        {
            throw std::runtime_error("Expected data type in key");
        }

        std::string str = (std::string)std::get<nlohmann::json>(next->data);

        int64_t offset_from_start = (uint8_t*)u - s->get_memory_base_ptr();
        int64_t mem_size = s->get_memory_base_size();

        int64_t slen = 0;

        for(int64_t i=offset_from_start; i < mem_size && slen < l; i++)
        {
            u[i - offset_from_start] = str[i - offset_from_start];

            slen++;
        }
    }
}

uint32_t gameapi_get_next_id(runtime::store* s)
{
    return s->interop_context.next_id++;
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
    runtime::externval serialise_length;

    runtime::externval get_next;

    serialise_begin.val = runtime::allochostsimplefunction<serialise_object_begin>(s);
    serialise_end.val = runtime::allochostsimplefunction<serialise_object_end>(s);

    serialise_begin_base.val = runtime::allochostsimplefunction<serialise_object_begin_base>(s);
    serialise_end_base.val = runtime::allochostsimplefunction<serialise_object_end_base>(s);

    serialise_u32.val = runtime::allochostsimplefunction<serialise_basic_u32>(s);
    serialise_u64.val = runtime::allochostsimplefunction<serialise_basic_u64>(s);
    serialise_float.val = runtime::allochostsimplefunction<serialise_basic_float>(s);
    serialise_double.val = runtime::allochostsimplefunction<serialise_basic_double>(s);
    serialise_string.val = runtime::allochostsimplefunction<serialise_basic_string>(s);
    serialise_length.val = runtime::allochostsimplefunction<serialise_basic_string_length>(s);

    get_next.val = runtime::allochostsimplefunction<gameapi_get_next_id>(s);

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
    vals["env"]["serialise_basic_string_length"] = serialise_length;

    vals["env"]["get_next_id"] = get_next;

    return vals;
}

