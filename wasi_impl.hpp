#ifndef WASI_IMPL_HPP_INCLUDED
#define WASI_IMPL_HPP_INCLUDED

#ifdef HOST

#define PTR(x) wasi_ptr_t<x>
#define DPTR(x) wasi_ptr_t<wasi_ptr_t<x>>

#else

#define wasi_size_t uint32_t

#define __wasi__

#define _Static_assert static_assert
#define _Noreturn [[noreturn]]
#define _Alignof alignof
#include <type_traits>

#include "core.h"

template<typename T>
struct wasi_ptr_t
{
    uint32_t val = 0;

    constexpr static bool is_ptr = true;

    wasi_ptr_t(uint32_t in)
    {
        val = in;
    }

    T* operator->()
    {
        assert(val + sizeof(T) <= mem_0.size());

        return (T*)&mem_0[val];
    }

    template<typename T1 = T, typename = std::enable_if<!std::is_same_v<T, void>>>
    T1& operator*()
    {
        assert(val + sizeof(T) <= mem_0.size());

        return *(T*)&mem_0[val];
    }
};

#define PTR(x) wasi_ptr_t<x>
#define DPTR(x) wasi_ptr_t<wasi_ptr_t<x>>

#endif // HOST

using wasi_ptr_raw = uint32_t;

__wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd, PTR(__wasi_prestat_t) buf)
{
    printf("Fd? %i\n", fd);
    return __WASI_EACCES;
}

__wasi_errno_t __wasi_fd_prestat_dir_name(__wasi_fd_t fd, PTR(char) path, wasi_size_t path_len)
{
    printf("FdDirName %i\n", fd);
    return __WASI_EACCES;
}

__wasi_errno_t __wasi_environ_sizes_get(PTR(wasi_size_t) environ_count, PTR(wasi_size_t) environ_buf_size)
{
    *environ_count = 0;
    *environ_buf_size = 0;
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_environ_get(wasi_ptr_t<wasi_ptr_t<char>> env, PTR(char) environ_buf)
{
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_sizes_get(PTR(wasi_size_t) argc, PTR(wasi_size_t) argv_buf_size)
{
    *argc = 0;
    *argv_buf_size = 0;
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_get(DPTR(char) argv, PTR(char) argv_buf)
{
    return __WASI_ESUCCESS;
}

void __wasi_proc_exit(__wasi_exitcode_t rval)
{
    printf("RVAL %i\n", rval);

    exit((int)rval);
}

__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, PTR(__wasi_fdstat_t) buf)
{
    __wasi_fdstat_t val;
    val.fs_filetype = __WASI_FILETYPE_UNKNOWN;
    val.fs_flags = __WASI_FDFLAG_SYNC;
    val.fs_rights_base = 0;
    val.fs_rights_inheriting = 0;

    *buf = val;

    return __WASI_ESUCCESS;
}

#ifndef HOST

#define EXPORT_0(x) uint32_t x(){return __wasi_##x;}
#define EXPORT_1(x, t1) uint32_t x(t1 v1){return __wasi_##x(v1);}
#define EXPORT_2(x, t1, t2) uint32_t x(t1 v1, t2 v2){return __wasi_##x(v1, v2);}
#define EXPORT_3(x, t1, t2, t3) uint32_t x(t1 v1, t2 v2, t3 v3){return __wasi_##x(v1, v2, v3);}

#define VEXPORT_1(x, t1) void x(t1 v1){return __wasi_##x(v1);}

EXPORT_2(fd_prestat_get, __wasi_fd_t, wasi_ptr_raw);
EXPORT_3(fd_prestat_dir_name, __wasi_fd_t, wasi_ptr_raw, wasi_size_t);
EXPORT_2(environ_sizes_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(environ_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(args_sizes_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(args_get, wasi_ptr_raw, wasi_ptr_raw);
VEXPORT_1(proc_exit, __wasi_exitcode_t);
EXPORT_2(fd_fdstat_get, __wasi_fd_t, wasi_ptr_raw);

#endif // HOST

#endif // WASI_IMPL_HPP_INCLUDED
