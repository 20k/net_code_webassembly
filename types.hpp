#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <array>
#include <cstring>
#include <variant>
#include <optional>
#include <assert.h>

namespace types
{
    ///u32, u64, s32, s64, i8, i16, i32, i64.

    template<typename T, typename derived>
    struct integral
    {
        T val = 0;

        using type = T;

        integral(const T& t) : val(t){}
        integral() = default;

        explicit operator T() const noexcept {return val;}

        derived& operator<<(int pval)
        {
            val = val << pval;

            return static_cast<derived&>(*this);
        }

        derived& operator|=(derived& other)
        {
            val = val | other.val;

            return static_cast<derived&>(*this);
        }

        derived& operator=(const derived& other) // copy assignment
        {
            if(this != &other)
            {
                val = other.val;
            }

            return static_cast<derived&>(*this);
        }

        derived& operator+(const derived& other) // copy assignment
        {
            val += other.val;

            return static_cast<derived&>(*this);
        }
    };

    struct i32 : integral<uint32_t, i32>{};
    struct u32 : integral<uint32_t, u32>{};
    struct s32 : integral<int32_t, s32>{};

    struct i64 : integral<uint64_t, i64>{};
    struct u64 : integral<uint64_t, u64>{};
    struct s64 : integral<int32_t, s64>{};

    struct i8 : integral<uint8_t, i8>{};
    struct i16 : integral<uint16_t, i16>{};

    struct f32 : integral<float, f32>{};
    struct f64 : integral<double, f64>{};

    template<typename T>
    struct vec
    {
        std::vector<T> v;

        template<typename U>
        vec(const U& v1, const U& v2) : v(v1, v2)
        {

        }

        vec() = default;

        vec(const std::initializer_list<T>& lst) : v(lst)
        {

        }

        template<int N>
        auto get() const
        {
            return v[N];
        }

        auto begin()
        {
            return v.begin();
        }

        auto end()
        {
            return v.end();
        }

        auto begin() const
        {
            return v.begin();
        }

        auto end() const
        {
            return v.end();
        }

        auto clear()
        {
            return v.clear();
        }

        auto size() const
        {
            return v.size();
        }

        void push_back(const T& t)
        {
            v.push_back(t);
        }

        template<typename... U>
        void emplace_back(U&&... t)
        {
            v.emplace_back(std::forward<U>(t)...);
        }

        auto pop_back()
        {
            return v.pop_back();
        }

        template<typename U, typename V>
        void insert(const U& v1, const V& v2, const V& v3)
        {
            v.insert(v1, v2, v3);
        }

        const T& front() const
        {
            return v.front();
        }

        T& front()
        {
            return v.front();
        }

        const T& back() const
        {
            return v.back();
        }

        T& back()
        {
            return v.back();
        }

        //template<typename U>
        inline
        const T& operator [](size_t i) const
        {
            //if(i >= (U)v.size() || i < 0)
            //    throw std::runtime_error("invalid bounds access");

            return v[i];
        }

        //template<typename U>
        inline
        T& operator [](size_t i)
        {
            //if(i >= (U)v.size() || i < 0)
            //    throw std::runtime_error("invalid bounds access");

            return v[i];
        }

        void resize(int n)
        {
            v.resize(n);
        }

        [[nodiscard]]
        vec<T> append(const vec<T>& dat) const
        {
            vec<T> ret;
            ret.v = v;

            for(size_t i=0; i < dat.size(); i++)
            {
                ret.push_back(dat[i]);
            }

            return ret;
        }

        template<typename U>
        void check(const U& u)
        {
            if(u >= (U)v.size() || u < 0)
                throw std::runtime_error("Not in bounds");
        }

        auto reserve(int in)
        {
            return v.reserve(in);
        }
    };

    template<typename T, int N>
    struct svec
    {
       // T* v = nullptr;

        std::array<T, N> v;
        uint32_t idx = 0;

        /*template<typename U>
        svec(const U& v1, const U& v2) : v(v1, v2)
        {

        }*/

        svec(const vec<T>& in)
        {
            v = new T[1024];

            for(size_t i = 0; i < in.size(); i++)
            {
                v[idx++] = in[i];
            }
        }

        svec()
        {

        }

        /*svec()
        {
            v = new T[1024];
        }

        ~svec()
        {
            delete [] v;
        }*/

        /*svec<T, N>& operator=(const svec<T, N>& other)
        {
            assert(false);
        }*/

        auto begin()
        {
            return &v[0];
        }

        auto end()
        {
            return &v[idx];
        }

        auto begin() const
        {
            return &v[0];
        }

        auto end() const
        {
            return &v[idx + 1];
        }

        void clear()
        {
            idx = 0;
        }

        auto size() const
        {
            return idx;
        }

        void push_back(const T& t)
        {
            v[idx++] = t;
            //v.push_back(t);
        }

        /*template<typename... U>
        void emplace_back(U&&... t)
        {
            v.emplace_back(std::forward<U>(t)...);
        }*/

        auto pop_back()
        {
            idx--;
            //return v.pop_back();
        }

        /*template<typename U, typename V>
        void insert(const U& v1, const V& v2, const V& v3)
        {
            //v.insert(v1, v2, v3);

            for(auto it = v2; it != v3; it++)
            {
                *v1 = *it;
            }
        }*/

        const T& front() const
        {
            return v[0];
        }

        T& front()
        {
            return v[0];
        }

        const T& back() const
        {
            return v[idx-1];
        }

        T& back()
        {
            return v[idx-1];
        }

        //template<typename U>
        inline
        const T& operator [](uint32_t i) const
        {
            //if(i >= (U)v.size() || i < 0)
            //    throw std::runtime_error("invalid bounds access");

            return v[i];
        }

        //template<typename U>
        inline
        T& operator [](uint32_t i)
        {
            //if(i >= (U)v.size() || i < 0)
            //    throw std::runtime_error("invalid bounds access");

            return v[i];
        }

        void resize(uint32_t n)
        {
            idx = n;
            //v.resize(n);
        }

        /*[[nodiscard]]
        vec<T> append(const vec<T>& dat) const
        {
            vec<T> ret;
            ret.v = v;

            for(size_t i=0; i < dat.size(); i++)
            {
                ret.push_back(dat[i]);
            }

            return ret;
        }*/

        template<typename U>
        void check(const U& u)
        {
            if(u >= (U)v.size() || u < 0)
                throw std::runtime_error("Not in bounds");
        }

        /*auto reserve(int in)
        {
            return v.reserve(in);
        }*/
    };

    /**0x7F -> i32
    0x7E -> i64
    0x7D -> f32
    0x7C -> f64*/

    ///which embeds one of the above constants
    struct valtype
    {
        uint8_t which = 0;

        /*union type
        {
            i32 i_32;
            i64 i_64;
            f32 f_32;
            f64 f_64;
        } val;*/

        std::string friendly() const
        {
            switch(which)
            {
                case 0x7F:
                    return "i32";
                case 0x7E:
                    return "i64";
                case 0x7D:
                    return "f32";
                case 0x7C:
                    return "f64";
                default:
                    return "i/fValTypeErr";
            }
        }

        void ci32()
        {
            which = 0x7F;
        }

        void ci64()
        {
            which = 0x7E;
        }

        template<typename T>
        void set()
        {
            if constexpr(std::is_same_v<T, types::i32>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, types::i64>)
            {
                which = 0x7E;
            }
            else if constexpr(std::is_same_v<T, types::f32>)
            {
                which = 0x7D;
            }
            else if constexpr(std::is_same_v<T, types::f64>)
            {
                which = 0x7C;
            }
            else if constexpr(std::is_same_v<T, bool>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, uint8_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, int8_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, uint16_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, int16_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, uint32_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, int32_t>)
            {
                which = 0x7F;
            }
            else if constexpr(std::is_same_v<T, uint64_t>)
            {
                which = 0x7E;
            }
            else if constexpr(std::is_same_v<T, int64_t>)
            {
                which = 0x7E;
            }
            else if constexpr(std::is_same_v<T, float>)
            {
                which = 0x7D;
            }
            else if constexpr(std::is_same_v<T, double>)
            {
                which = 0x7C;
            }
            else if constexpr(std::is_pointer_v<T>)
            {
                ///32bit
                which = 0x7F;
            }
        }
    };

    inline
    std::string friendly(uint32_t)
    {
        return "i32";
    }

    inline
    std::string friendly(uint64_t)
    {
        return "i64";
    }

    inline
    std::string friendly(int32_t)
    {
        return "i32";
    }

    inline
    std::string friendly(int64_t)
    {
        return "i64";
    }

    inline
    std::string friendly(float)
    {
        return "f32";
    }

    inline
    std::string friendly(double)
    {
        return "f64";
    }

    inline
    std::string friendly(types::i32)
    {
        return "i32";
    }

    inline
    std::string friendly(types::i64)
    {
        return "i64";
    }

    inline
    std::string friendly(types::f32)
    {
        return "f32";
    }

    inline
    std::string friendly(types::f64)
    {
        return "f64";
    }

    struct blocktype
    {
        uint8_t which = 0;

        std::string friendly() const
        {
            switch(which)
            {
                case 0x7F:
                    return "i32";
                case 0x7E:
                    return "i64";
                case 0x7D:
                    return "f32";
                case 0x7C:
                    return "f64";
                case 0x40:
                    return "empty";
                default:
                    return "i/fValTypeErr";
            }
        }

        int arity() const
        {
            return which != 0x40;
        }
    };

    struct functype
    {
        vec<valtype> params;
        vec<valtype> results;

        std::string as_string()
        {
            std::string s = "P: ";

            for(int i=0; i < (int)params.size(); i++)
            {
                s += params[i].friendly() + ", ";
            }

            s += "R: ";

            for(int i=0; i < (int)results.size(); i++)
            {
                s += results[i].friendly() + ", ";
            }

            return s;
        }
    };

    inline
    bool funcs_equal(const functype& t1, const functype& t2)
    {
        if(t1.params.size() != t2.params.size())
            return false;

        if(t1.results.size() != t2.results.size())
            return false;

        for(size_t i=0; i < t1.params.size(); i++)
        {
            if(t1.params[i].which != t2.params[i].which)
                return false;
        }

        for(size_t i=0; i < t1.results.size(); i++)
        {
            if(t1.results[i].which != t2.results[i].which)
                return false;
        }

        return true;
    }

    struct name
    {
        vec<uint8_t> dat;

        std::string friendly() const
        {
            std::string ret(dat.v.begin(), dat.v.end());

            return ret;
        }

        bool operator==(const std::string& in)
        {
            if((int)in.size() != (int)dat.size())
                return false;

            for(int i=0; i < (int)dat.size(); i++)
            {
                if(dat[i] != in[i])
                    return false;
            }

            return true;
        }
    };

    struct typeidx : u32{};
    struct funcidx : u32{};
    struct tableidx : u32{};
    struct memidx : u32{};

    struct globalidx : u32{};
    struct localidx : u32{};
    struct labelidx : u32{};

    ///only valid is 0x70 -> anyfunc
    struct elemtype
    {
        uint8_t which = 0;

        std::string friendly()
        {
            if(which == 0x70)
                return "anyfunc";

            return "ErrElementType";
        }
    };

    struct limits
    {
        uint8_t has_max_val = 0;

        u32 n;
        u32 m;

        bool has_max()
        {
            return has_max_val;
        }

        std::optional<u32> get_max() const
        {
            if(has_max_val)
                return m;

            return std::nullopt;
        }
    };

    struct tabletype
    {
        elemtype et;
        limits lim;
    };

    struct memtype
    {
        limits lim;
    };

    struct mut
    {
        uint8_t is_mut = 0;

        std::string friendly()
        {
            if(is_mut == 0)
                return "const";
            if(is_mut == 1)
                return "var";

            return "ErrMut";
        }
    };

    struct globaltype
    {
        valtype type;
        mut m;
    };

    struct externtype
    {
        std::variant<functype, tabletype, memtype, globaltype> type;
    };

    struct table
    {
        tabletype type;
    };

    struct mem
    {
        memtype type;
    };

    struct memarg
    {
        u32 align;
        u32 offset;
    };

    struct br_table_data
    {
        vec<labelidx> labels;
        labelidx fin;
    };

    struct instr;

    struct double_branch_data
    {
        blocktype btype;

        bool has_second_branch = false;

        vec<instr> first;
        vec<instr> second;
    };

    struct single_branch_data
    {
        blocktype btype;

        vec<instr> first;
    };

    struct instr
    {
        //static inline int assert_on_destruct = 0;

        size_t which = 0;

        /*union constant
        {
            i32 i_32;
            i64 i_64;
            f32 f_32;
            f64 f_64;

            memarg arg;

            globalidx gidx;
            localidx lidx;

            typeidx tidx;
            funcidx fidx;

            br_table_data br_td;

            double_branch_data dbd;
            single_branch_data sbd;
        } cst;*/

        std::variant<i32, i64, f32, f64, memarg, globalidx, localidx, labelidx, typeidx, funcidx, br_table_data, double_branch_data, single_branch_data> dat;

        #if 0
        instr()
        {

        }

        ~instr()
        {
            //printf("destruct\n");

            /*if(assert_on_destruct)
                assert(false);*/
        }
        #endif // 0
    };

    struct expr
    {
        vec<instr> i;
    };

    struct global
    {
        globaltype type;
        expr e;
    };

    struct local
    {
        u32 n;
        valtype type;
    };

    struct code_func
    {
        vec<local> locals;
        expr e;
    };

    struct code
    {
        u32 size;
        code_func fnc;
    };

    struct dataseg
    {
        memidx x;
        expr e;
        types::vec<uint8_t> b;
    };
}

struct data
{
    std::vector<uint8_t> ptr;
    int32_t offset = 0;

    uint8_t next()
    {
        if(offset >= (int32_t)ptr.size())
            throw std::runtime_error("offset >= ptr.size()");

        uint8_t val = ptr[offset];

        offset++;

        return val;
    }

    int32_t size()
    {
        return ptr.size();
    }

    void push_back(uint8_t val)
    {
        ptr.push_back(val);
    }

    void load_from_data(const std::string& data)
    {
        ptr = std::vector<uint8_t>(data.begin(), data.end());
    }

    void load_from_file(const std::string& wasm)
    {
        std::ifstream t(wasm, std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(t)),
                                   std::istreambuf_iterator<char>());

        if(!t.good())
            throw std::runtime_error("Could not open file " + wasm);

        ptr = data;
    }
};

#endif // TYPES_HPP_INCLUDED
