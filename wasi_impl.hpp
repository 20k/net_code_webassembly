#ifndef WASI_IMPL_HPP_INCLUDED
#define WASI_IMPL_HPP_INCLUDED

#ifdef HOST

#else
#include <wasi/core.h>
#endif // HOST

__wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd, __wasi_prestat_t* buf)
{
    printf("Fd? %i\n", fd);
    return __WASI_EACCES;
}

__wasi_errno_t __wasi_fd_prestat_dir_name(__wasi_fd_t fd, char* path, wasi_size_t path_len)
{
    printf("FdDirName %i\n", fd);
    return __WASI_EACCES;
}

__wasi_errno_t __wasi_environ_sizes_get(wasi_size_t* environ_count, wasi_size_t* environ_buf_size)
{
    *environ_count = 0;
    *environ_buf_size = 0;
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_environ_get(char** environ, char* environ_buf)
{
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_sizes_get(wasi_size_t* argc, wasi_size_t* argv_buf_size)
{
    *argc = 0;
    *argv_buf_size = 0;
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_get(char** argv, char* argv_buf)
{
    return __WASI_ESUCCESS;
}

void __wasi_proc_exit(__wasi_exitcode_t rval)
{
    exit((int)rval);
}

__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* buf)
{
    __wasi_fdstat_t val;
    val.fs_filetype = __WASI_FILETYPE_UNKNOWN;
    val.fs_flags = __WASI_FDFLAG_SYNC;
    val.fs_rights_base = 0;
    val.fs_rights_inheriting = 0;

    *buf = val;

    return __WASI_ESUCCESS;
}

#endif // WASI_IMPL_HPP_INCLUDED
