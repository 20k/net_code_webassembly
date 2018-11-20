#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <array>
#include <cstring>
#include <variant>

namespace types
{
    ///u32, u64, s32, s64, i8, i16, i32, i64.

    template<typename T, typename derived>
    struct integral
    {
        T val = 0;

        integral(const T& t) : val(t){}
        integral(){}

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

        int32_t size()
        {
            return v.size();
        }

        void push_back(const T& t)
        {
            v.push_back(t);
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
    };

    struct func
    {
        vec<valtype> params;
        vec<valtype> results;
    };

    struct name
    {
        vec<uint8_t> dat;

        std::string friendly()
        {
            std::string ret(dat.v.begin(), dat.v.end());

            return ret;
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

    ///currently totally discards all information relating to instructions so i can get through basic parsing
    ///later this will be pretty humongous
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

        std::variant<i32, i64, f32, f64, memarg, globalidx, localidx, typeidx, funcidx, br_table_data, double_branch_data, single_branch_data> dat;
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
