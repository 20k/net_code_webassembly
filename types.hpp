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

namespace types
{
    ///u32, u64, s32, s64, i8, i16, i32, i64.

    template<typename T, typename derived>
    struct integral
    {
        T val = 0;

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

        int32_t size() const
        {
            return v.size();
        }

        void push_back(const T& t)
        {
            v.push_back(t);
        }

        template<typename U>
        void emplace_back(const U& t)
        {
            v.emplace_back(t);
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

        T back() const
        {
            return v.back();
        }

        T& back()
        {
            return v.back();
        }

        template<typename U>
        inline
        T operator [](const U& i) const
        {
            //if(i >= (U)v.size() || i < 0)
            //    throw std::runtime_error("invalid bounds access");

            return v[i];
        }

        template<typename U>
        inline
        T& operator [](const U& i)
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

            for(int i=0; i < dat.size(); i++)
            {
                ret.push_back(dat[i]);
            }

            return ret;
        }
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

        std::string friendly()
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
    };

    struct blocktype
    {
        uint8_t which = 0;

        std::string friendly()
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

        int arity()
        {
            return which != 0x40;
        }
    };

    struct functype
    {
        vec<valtype> params;
        vec<valtype> results;
    };

    inline
    bool funcs_equal(const functype& t1, const functype& t2)
    {
        if(t1.params.size() != t2.params.size())
            return false;

        if(t1.results.size() != t2.results.size())
            return false;

        for(int i=0; i < t1.params.size(); i++)
        {
            if(t1.params[i].which != t2.params[i].which)
                return false;
        }

        for(int i=0; i < t1.results.size(); i++)
        {
            if(t1.results[i].which != t2.results[i].which)
                return false;
        }

        return true;
    }

    struct name
    {
        vec<uint8_t> dat;

        std::string friendly()
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
        uint8_t which = 0;

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
