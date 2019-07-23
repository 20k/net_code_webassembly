#ifndef RUNTIME_TYPES_HPP_INCLUDED
#define RUNTIME_TYPES_HPP_INCLUDED

#include "types.hpp"
#include <optional>
#include <functional>
#include "template_args_helper.hpp"
#include "wasm_interop_context.hpp"

struct module;

namespace runtime
{
    struct addr : types::integral<uint32_t, addr>{};

    struct funcaddr : addr {friend bool operator<(const funcaddr& one, const funcaddr& two);};
    struct tableaddr : addr {};
    struct memaddr : addr {};
    struct globaladdr : addr {};

    inline
    bool operator<(const runtime::funcaddr& one, const runtime::funcaddr& two)
    {
        return one.val < two.val;
    }

    struct externval
    {
        std::variant<funcaddr, tableaddr, memaddr, globaladdr> val;
    };

    struct exportinst
    {
        types::name name;
        externval value;
    };

    struct webasm_func
    {
        types::code funct;
    };

    struct store;
    struct value;

    struct host_func
    {
        //void(*ptr)(void);
        std::function<std::optional<runtime::value>(const types::vec<runtime::value>&, runtime::store*)> ptr;
    };

    ///uuh
    ///this probably should be a pointer or something to moduleinst
    struct funcinst
    {
        types::functype type;
        //moduleinst module;
        ///do this implicitly at runtime?

        std::variant<host_func, webasm_func> funct;
    };

    struct funcelem
    {
        std::optional<funcaddr> addr;
    };

    struct tableinst
    {
        types::vec<funcelem> elem;
        std::optional<types::u32> max;
    };

    struct meminst
    {
        types::vec<uint8_t> dat;
        std::optional<types::u32> max;
    };

    struct store;

    ///hmm
    ///might be better to hold a uint64_t
    ///and then just cast on the fly
    ///using a union is not better apparently
    struct value
    {
        std::variant<types::i32, types::i64, types::f32, types::f64> v;

        void from_valtype(const types::valtype& type)
        {
            switch(type.which)
            {
                case 0x7F:
                    v = types::i32{0};
                    return;
                case 0x7E:
                    v = types::i64{0};
                    return;
                case 0x7D:
                    v = types::f32{0};
                    return;
                case 0x7C:
                    v = types::f64{0};
                    return;
                default:
                    throw std::runtime_error("Invalid value in which");
            }
        }

        template<typename T>
        value(const T& in)
        {
            set(in);
        }

        value() = default;

        void set(uint32_t t)
        {
            v = types::i32{t};
        }

        void set(int32_t t)
        {
            v = types::i32{(uint32_t)t};
        }

        void set(uint64_t t)
        {
            v = types::i64{t};
        }

        void set(int64_t t)
        {
            v = types::i64{(uint64_t)t};
        }

        void set(float t)
        {
            v = types::f32{t};
        }

        void set(double t)
        {
            v = types::f64{t};
        }

        void set(const types::i32& in)
        {
            v = in;
        }

        void set(const types::i64& in)
        {
            v = in;
        }

        void set(const types::f32& in)
        {
            v = in;
        }

        void set(const types::f64& in)
        {
            v = in;
        }

        /*explicit operator uint32_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator int32_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator uint16_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator int16_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator uint8_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator int8_t() const noexcept {return std::get<types::i32>(v).val;}
        explicit operator bool() const noexcept {return std::get<types::i32>(v).val;}

        explicit operator uint64_t() const noexcept {return std::get<types::i64>(v).val;}
        explicit operator int64_t() const noexcept {return std::get<types::i64>(v).val;}

        explicit operator float() const noexcept {return std::get<types::f32>(v).val;}
        explicit operator double() const noexcept {return std::get<types::f64>(v).val;}*/

        template<typename T>
        auto apply(const T& t) const
        {
            if(std::holds_alternative<types::i32>(v))
                return t(std::get<types::i32>(v).val);
            else if(std::holds_alternative<types::i64>(v))
                return t(std::get<types::i64>(v).val);
            else if(std::holds_alternative<types::f32>(v))
                return t(std::get<types::f32>(v).val);
            else if(std::holds_alternative<types::f64>(v))
                return t(std::get<types::f64>(v).val);

            throw std::runtime_error("nope");
        }

        std::string friendly_val() const
        {
            return apply([](auto concrete){return std::to_string(concrete);});
        }

        std::string friendly() const
        {
            if(std::holds_alternative<types::i32>(v))
                return "i32";
            else if(std::holds_alternative<types::i64>(v))
                return "i64";
            else if(std::holds_alternative<types::f32>(v))
                return "f32";
            else if(std::holds_alternative<types::f64>(v))
                return "f64";
            else
                return "empty";
        }

        bool is_i32()
        {
            return std::holds_alternative<types::i32>(v);
        }
    };

    struct globalinst;
    struct moduleinst;

    struct store
    {
        types::vec<funcinst> funcs;
        types::vec<tableinst> tables;
        types::vec<meminst> mems;
        types::vec<globalinst> globals;

        wasm_interop_context interop_context;

        funcaddr allocfunction(const module& m, size_t idx);
        funcaddr allochostfunction(const types::functype& type, const std::function<std::optional<runtime::value>(const types::vec<runtime::value>&, runtime::store* s)>& ptr);

        tableaddr alloctable(const types::tabletype& type);
        memaddr allocmem(const types::memtype& type);
        globaladdr allocglobal(const types::globaltype& type, const value& v);

        types::vec<runtime::value> invoke(const funcaddr& address, moduleinst& minst, const types::vec<value>& vals);
        types::vec<runtime::value> invoke_by_name(const std::string& imported, moduleinst& minst, const types::vec<value>& vals);

        uint8_t* get_memory_base_ptr();
        uint32_t get_memory_base_size();
    };

    struct wasi_ctx
    {
        static inline thread_local runtime::store* cstore = nullptr;
    };

    template<typename T>
    struct wasi_ptr_t
    {
        uint32_t val = 0;

        constexpr static bool is_ptr = true;

        wasi_ptr_t()
        {

        }

        wasi_ptr_t(uint32_t in)
        {
            val = in;
        }

        T* operator->() const
        {
            assert(wasi_ctx::cstore);

            wasi_ctx::cstore->mems[0].dat.check(val);
            wasi_ctx::cstore->mems[0].dat.check(val + sizeof(T));

            return (T*)&wasi_ctx::cstore->mems[0].dat[val];
        }

        template<typename T1 = T, typename = std::enable_if<!std::is_same_v<T, void>>>
        T1& operator*() const
        {
            assert(wasi_ctx::cstore);

            wasi_ctx::cstore->mems[0].dat.check(val);
            wasi_ctx::cstore->mems[0].dat.check(val + sizeof(T));

            return *(T*)&wasi_ctx::cstore->mems[0].dat[val];
        }

        template<typename T1 = T, typename = std::enable_if<!std::is_same_v<T, void>>>
        T1& operator[](int idx) const
        {
            assert(val + idx * sizeof(T) + sizeof(T) <= wasi_ctx::cstore->mems[0].dat.size());
            assert(val + idx >= 0);

            return *(T*)&wasi_ctx::cstore->mems[0].dat[val + idx * sizeof(T)];
        }
    };

    /*template<typename T>
    struct is_wasi_ptr
    {
        template<typename U>
        static constexpr decltype(std::declval<U>()::is_ptr, bool())
        test_ptr(int)
        {
            return true;
        }

        template<typename U>
        static constexpr bool test_ptr(...)
        {
            return false;
        }

        static constexpr bool value = test_ptr<T>(int());
    };*/

    template<typename T, typename = int>
    struct is_wasi_ptr : std::false_type {};

    template<typename T>
    struct is_wasi_ptr<T, decltype((void)T::is_ptr, 0)> : std::true_type{};


    template<typename T>
    T get(const runtime::value& v, runtime::meminst& minst, T dummy)
    {
        if constexpr(std::is_same_v<T, uint32_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, int32_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, uint16_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, int16_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, uint8_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, int8_t>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, bool>)
        {
            return std::get<types::i32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, uint64_t>)
        {
            return std::get<types::i64>(v.v).val;
        }

        if constexpr(std::is_same_v<T, int64_t>)
        {
            return std::get<types::i64>(v.v).val;
        }

        if constexpr(std::is_same_v<T, float>)
        {
            return std::get<types::f32>(v.v).val;
        }

        if constexpr(std::is_same_v<T, double>)
        {
            return std::get<types::f64>(v.v).val;
        }
    }

    template<typename T>
    T* get(const runtime::value& v, runtime::meminst& minst, T* dummy)
    {
        uint32_t ptr = get(v, minst, uint32_t());

        if(ptr + sizeof(T) > minst.dat.size())
            throw std::runtime_error("Ptr out of bounds");

        return (T*)&minst.dat[ptr];
    }

    template<typename T>
    wasi_ptr_t<T> get(const runtime::value& v, runtime::meminst& minst, wasi_ptr_t<T> dummy)
    {
        uint32_t ptr = get(v, minst, uint32_t());

        if(ptr + sizeof(T) > minst.dat.size())
            throw std::runtime_error("Ptr out of bounds");

        wasi_ptr_t<T> ret;
        ret.val = ptr;

        return ret;
    }

    namespace detail
    {
        template<typename T, typename... U>
        inline
        constexpr bool has_runtime(T(*func)(U... args))
        {
            int iruntime_c = 0;

            ((iruntime_c += (int)std::is_same_v<U, runtime::store*>), ...);

            return iruntime_c > 0;
        }

        template<typename T>
        void type_push(types::vec<types::valtype>& in)
        {
            types::valtype type;

            if(std::is_same_v<T, void>)
                return;

            if(std::is_same_v<T, runtime::store*>)
                return;

            if(is_wasi_ptr<T>())
            {
                type.set<types::i32>();
            }
            else
            {
                type.set<T>();
            }

            in.push_back(type);
        }

        template<typename T, typename... U>
        inline
        constexpr types::functype get_functype(T(*func)(U... args))
        {
            types::functype ret;

            ((type_push<U>(ret.params)), ...);

            type_push<T>(ret.results);

            return ret;
        }

        template<typename tup, std::size_t... Is>
        void set_args(tup& t, const types::vec<runtime::value>& v, std::index_sequence<Is...>, runtime::store* s)
        {
            //std::tuple_element_t<Is, tup>

            //.get(s, std::tuple_element_t<Is, tup>())

            ((std::get<Is>(t) = runtime::get(v.get<Is>(), s->mems[0], std::tuple_element_t<Is, tup>())), ...);
        }

        template<typename V, V& v, typename return_type, typename... args_type>
        std::optional<runtime::value> host_shim_impl(const types::vec<runtime::value>& vals, runtime::store* s)
        {
            std::tuple<args_type...> args;

            std::index_sequence_for<args_type...> iseq;

            set_args(args, vals, iseq, s);

            if constexpr(std::is_same_v<return_type, void>)
            {
                std::apply(v, args);
                return std::nullopt;
            }

            if constexpr(!std::is_same_v<return_type, void>)
            {
                return std::apply(v, args);
            }
        }

        template<auto& v, typename T, typename... U>
        auto host_shim(T(*func)(U... args))
        {
            return &host_shim_impl<decltype(v), v, T, U...>;
        }

        template<auto& t>
        constexpr auto base_shim()
        {
            return host_shim<t>(t);
        }

        template<typename V, V& v, typename return_type, typename rstore, typename... args_type>
        std::optional<runtime::value> host_shim_impl_with_runtime(const types::vec<runtime::value>& vals, runtime::store* s)
        {
            std::tuple<args_type...> args;

            std::index_sequence_for<args_type...> iseq;

            set_args(args, vals, iseq, s);

            std::tuple<runtime::store*> first;
            std::get<0>(first) = s;

            if constexpr(std::is_same_v<return_type, void>)
            {
                std::apply(v, std::tuple_cat(first, args));
                return std::nullopt;
            }

            if constexpr(!std::is_same_v<return_type, void>)
            {
                return std::apply(v, std::tuple_cat(first, args));
            }
        }

        template<auto& v, typename T, typename... U>
        auto host_shim_with_runtime(T(*func)(U... args))
        {
            return &host_shim_impl_with_runtime<decltype(v), v, T, U...>;
        }

        template<auto& t>
        constexpr auto base_shim_with_runtime()
        {
            return host_shim_with_runtime<t>(t);
        }
    }

    inline
    bool same_type(const value& v1, const value& v2)
    {
        if(std::holds_alternative<types::i32>(v1.v) && std::holds_alternative<types::i32>(v2.v))
            return true;
        if(std::holds_alternative<types::i64>(v1.v) && std::holds_alternative<types::i64>(v2.v))
            return true;
        if(std::holds_alternative<types::f32>(v1.v) && std::holds_alternative<types::f32>(v2.v))
            return true;
        if(std::holds_alternative<types::f64>(v1.v) && std::holds_alternative<types::f64>(v2.v))
            return true;

        return false;
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u, const value& v)
    {
        if(!same_type(u, v))
            throw std::runtime_error("Not same type");

        if(std::holds_alternative<types::i32>(u.v))
            return t(std::get<types::i32>(u.v).val, std::get<types::i32>(v.v).val);

        else if(std::holds_alternative<types::i64>(u.v))
            return t(std::get<types::i64>(u.v).val, std::get<types::i64>(v.v).val);

        else if(std::holds_alternative<types::f32>(u.v))
            return t(std::get<types::f32>(u.v).val, std::get<types::f32>(v.v).val);

        else if(std::holds_alternative<types::f64>(u.v))
            return t(std::get<types::f64>(u.v).val, std::get<types::f64>(v.v).val);

        else
            throw std::runtime_error("apply bad type");

        //return t(u, v);
    }

    template<typename T>
    inline
    auto apply(const T& t, const value& u)
    {
        if(std::holds_alternative<types::i32>(u.v))
            return t(std::get<types::i32>(u.v).val);

        else if(std::holds_alternative<types::i64>(u.v))
            return t(std::get<types::i64>(u.v).val);

        else if(std::holds_alternative<types::f32>(u.v))
            return t(std::get<types::f32>(u.v).val);

        else if(std::holds_alternative<types::f64>(u.v))
            return t(std::get<types::f64>(u.v).val);

        else
            throw std::runtime_error("apply bad type");

        //return t(u, v);
    }

    struct globalinst
    {
        value val;
        types::mut mut;
    };

    struct moduleinst;

    ///create a wrapper for allochostfunction which deduces type and automatically creates a shim

    ///ok so we're up to simple arg values now
    ///need to automatically shim the vectorof values we get to the input function when we call it
    ///not 100% sure how to do that
    template<auto& t>
    funcaddr allochostsimplefunction(runtime::store& s)
    {
        types::functype type = detail::get_functype(t);

        if constexpr(!detail::has_runtime(t))
        {
            auto shim = detail::base_shim<t>();

            return s.allochostfunction(type, shim);
        }

        if constexpr(detail::has_runtime(t))
        {
            auto shim = detail::base_shim_with_runtime<t>();

            return s.allochostfunction(type, shim);
        }
    }

    template<typename T>
    inline
    types::vec<T> filter_type(const types::vec<externval>& vals)
    {
        types::vec<T> ret;

        for(const externval& val : vals)
        {
            if(std::holds_alternative<T>(val.val))
            {
                ret.push_back(std::get<T>(val.val));
            }
        }

        return ret;
    }

    inline
    types::vec<funcaddr> filter_func(const types::vec<externval>& vals)
    {
        return filter_type<funcaddr>(vals);
    }

    inline
    types::vec<tableaddr> filter_table(const types::vec<externval>& vals)
    {
        return filter_type<tableaddr>(vals);
    }

    inline
    types::vec<memaddr> filter_mem(const types::vec<externval>& vals)
    {
        return filter_type<memaddr>(vals);
    }

    inline
    types::vec<globaladdr> filter_global(const types::vec<externval>& vals)
    {
        return filter_type<globaladdr>(vals);
    }

    struct preserved_data_segment
    {
        uint32_t do_i = 0;
        uint32_t dend = 0;

        types::vec<uint8_t> bytes;
    };

    struct func_descriptor
    {
        types::typeidx tidx;
        std::string name;
        std::string module;
    };

    ///so this is constructed from our module
    ///which is the section representation we constructed earlier
    struct moduleinst
    {
        types::vec<types::functype> typel;

        types::vec<funcaddr> funcaddrs;
        types::vec<tableaddr> tableaddrs;
        types::vec<memaddr> memaddrs;
        types::vec<globaladdr> globaladdrs;

        std::map<funcaddr, func_descriptor> funcdescs;

        types::vec<exportinst> exports;

        types::vec<preserved_data_segment> data_segment;
    };

    constexpr size_t page_size = 64*1024;
    ///128 MB
    constexpr size_t sandbox_mem_cap = 128 * 1024 * 1024;
}

/*explicit operator uint32_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator int32_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator uint16_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator int16_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator uint8_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator int8_t() const noexcept {return std::get<types::i32>(v).val;}
explicit operator bool() const noexcept {return std::get<types::i32>(v).val;}

explicit operator uint64_t() const noexcept {return std::get<types::i64>(v).val;}
explicit operator int64_t() const noexcept {return std::get<types::i64>(v).val;}

explicit operator float() const noexcept {return std::get<types::f32>(v).val;}
explicit operator double() const noexcept {return std::get<types::f64>(v).val;}*/

/*template<>
uint32_t runtime::value::get<uint32_t>(runtime::store* s)
{
    return std::get<types::i32>(v).val;
}*/

#endif // RUNTIME_TYPES_HPP_INCLUDED
