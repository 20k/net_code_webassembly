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


template<typename T>
struct wasi_ptr_t
{
    uint32_t val = 0;

    constexpr static bool is_ptr = true;

    wasi_ptr_t(uint32_t in)
    {
        val = in;
    }

    template<typename = std::enable_if<std::is_same_v<T, void>>>
    wasi_ptr_t(const T* in)
    {
        assert(mem_0.size() > 0);

        val = (const char*)in - (const char*)&mem_0[0];
    }

    T* operator->() const
    {
        assert(val + sizeof(T) <= mem_0.size());

        return (T*)&mem_0[val];
    }

    template<typename T1 = T, typename = std::enable_if<!std::is_same_v<T, void>>>
    T1& operator*() const
    {
        assert(val + sizeof(T) <= mem_0.size());

        return *(T*)&mem_0[val];
    }

    template<typename T1 = T, typename = std::enable_if<!std::is_same_v<T, void>>>
    T1& operator[](int idx) const
    {
        assert(val + idx * sizeof(T) + sizeof(T) <= mem_0.size());
        assert(val + idx >= 0);

        return *(T*)&mem_0[val + idx * sizeof(T)];
    }
};

#define CLIENT
#include "wasi_host.hpp"

#define PTR(x) wasi_ptr_t<x>
#define DPTR(x) wasi_ptr_t<wasi_ptr_t<x>>

#endif // HOST

using wasi_ptr_raw = uint32_t;

#include <map>
#include <filesystem>

/*struct file_desc
{

};

struct fd_table
{
    __wasi_fd_t glob = 2;

    std::map<__wasi_fd_t, file_desc> files;

    __wasi_fd_t open()
    {
        files[glob++] = file_desc();

        return glob-1;
    }

    bool is_open(__wasi_fd_t fd)
    {
        return files.find(fd) != files.end();
    }

    void close(__wasi_fd_t fd)
    {
        files.erase(fd);
    }
};*/

struct preopened
{
    std::vector<std::filesystem::path> paths;

    preopened()
    {
        paths.push_back(std::filesystem::path("./sandbox"));
    }
};

/*fd_table global_table;

void boot_fds()
{
    global_table.open();
}*/

preopened file_sandbox;

__wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd, PTR(__wasi_prestat_t) buf)
{
    printf("Fd? %i\n", fd);

    if(file_sandbox.paths.size() == 0)
        return __WASI_EBADF;

    ///some baffling platform stuff
    if(file_sandbox.paths.size() == 1 && fd == 3)
    {
        __wasi_prestat_t ret;
        ret.pr_type = __WASI_PREOPENTYPE_DIR;

        __wasi_prestat_t::__wasi_prestat_u::__wasi_prestat_u_dir_t dird;
        dird.pr_name_len = file_sandbox.paths[0].string().length();

        ret.u.dir = dird;

        *buf = ret;

        return __WASI_ESUCCESS;
    }

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_fd_prestat_dir_name(__wasi_fd_t fd, PTR(char) path, wasi_size_t path_len)
{
    printf("FdDirName %i %i\n", fd, path_len);

    if(file_sandbox.paths.size() == 1 && fd == 3)
    {
        std::string cname = file_sandbox.paths[0].string();

        for(int i=0; i < cname.size() && i < path_len; i++)
        {
            path[i] = cname[i];
        }

        return __WASI_ESUCCESS;
    }

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_environ_sizes_get(PTR(wasi_size_t) environ_count, PTR(wasi_size_t) environ_buf_size)
{
    printf("ESize\n");

    *environ_count = 0;
    *environ_buf_size = 0;
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_environ_get(wasi_ptr_t<wasi_ptr_t<char>> env, PTR(char) environ_buf)
{
    printf("EGet\n");

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_sizes_get(PTR(wasi_size_t) argc, PTR(wasi_size_t) argv_buf_size)
{
    printf("SSize\n");

    *argc = 1;
    *argv_buf_size = strlen("sbox");
    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_args_get(DPTR(char) argv, PTR(char) argv_buf)
{
    printf("SGet\n");

    const char* name = "sbox";

    for(int i=0; i < strlen(name); i++)
    {
        argv_buf[i] = name[i];
    }

    (*argv).val = argv_buf.val;

    return __WASI_ESUCCESS;
}

void __wasi_proc_exit(__wasi_exitcode_t rval)
{
    printf("RVAL %i\n", rval);

    exit((int)rval);
}

__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, PTR(__wasi_fdstat_t) buf)
{
    printf("FdStat\n");

    if(file_sandbox.paths.size() == 1 && fd == 3)
    {
        printf("Yaystat\n");

        __wasi_fdstat_t preval;

        preval.fs_filetype = __WASI_FILETYPE_DIRECTORY;
        preval.fs_flags = __WASI_FDFLAG_SYNC;
        preval.fs_rights_base = __WASI_RIGHT_FD_DATASYNC |
                                __WASI_RIGHT_FD_READ |
                                __WASI_RIGHT_FD_SEEK |
                                __WASI_RIGHT_FD_FDSTAT_SET_FLAGS |
                                __WASI_RIGHT_FD_SYNC |
                                __WASI_RIGHT_FD_TELL |
                                __WASI_RIGHT_FD_WRITE |
                                __WASI_RIGHT_FD_ADVISE |
                                __WASI_RIGHT_FD_ALLOCATE |
                                __WASI_RIGHT_PATH_CREATE_DIRECTORY |
                                __WASI_RIGHT_PATH_CREATE_FILE |
                                __WASI_RIGHT_PATH_OPEN |
                                __WASI_RIGHT_FD_READDIR |
                                __WASI_RIGHT_PATH_READLINK |
                                __WASI_RIGHT_PATH_FILESTAT_GET |
                                __WASI_RIGHT_PATH_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_PATH_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_FD_FILESTAT_GET |
                                __WASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_FD_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_PATH_UNLINK_FILE |
                                __WASI_RIGHT_PATH_REMOVE_DIRECTORY |
                                __WASI_RIGHT_POLL_FD_READWRITE;

        preval.fs_rights_inheriting = preval.fs_rights_base;

        *buf = preval;

        return __WASI_ESUCCESS;
    }

    __wasi_fdstat_t val;
    val.fs_filetype = __WASI_FILETYPE_UNKNOWN;
    val.fs_flags = __WASI_FDFLAG_SYNC;
    val.fs_rights_base = 0;
    val.fs_rights_inheriting = 0;

    *buf = val;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd)
{
    printf("Close\n");

    if(fd == 0 || fd == 1 || fd == 2)
        return __WASI_EBADF;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, PTR(__wasi_filesize_t) newoffset)
{
    printf("Seek\n");

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd, const wasi_ptr_t<__wasi_ciovec_t> iovs, wasi_size_t iovs_len, wasi_ptr_t<uint32_t> nwritten)
{
    printf("Write %i\n", fd);

    auto do_op = [&](auto func)
    {
        int written = 0;

        for(int i=0; i < (int)iovs_len; i++)
        {
            __wasi_ciovec_t single = iovs[i];

            const wasi_ptr_t<void> buf = single.buf;

            wasi_ptr_t<char> tchr(0);
            tchr.val = buf.val;

            for(int kk=0; kk < single.buf_len; kk++)
            {
                func(tchr[kk]);
                written++;
            }
        }

        return written;
    };

    ///stdin
    if(fd == 0)
    {

    }

    ///stdout
    if(fd == 1)
    {
        *nwritten = do_op([](char in){return fputc(in, stdout);});
        return __WASI_ESUCCESS;
    }

    ///stderr
    if(fd == 2)
    {
        *nwritten = do_op([](char in){return fputc(in, stderr);});
        return __WASI_ESUCCESS;
    }

    *nwritten = 0;
    return __WASI_EBADF;
}

#ifndef HOST

#define EXPORT_0(x) uint32_t x(){return __wasi_##x;}
#define EXPORT_1(x, t1) uint32_t x(t1 v1){return __wasi_##x(v1);}
#define EXPORT_2(x, t1, t2) uint32_t x(t1 v1, t2 v2){return __wasi_##x(v1, v2);}
#define EXPORT_3(x, t1, t2, t3) uint32_t x(t1 v1, t2 v2, t3 v3){return __wasi_##x(v1, v2, v3);}
#define EXPORT_4(x, t1, t2, t3, t4) uint32_t x(t1 v1, t2 v2, t3 v3, t4 v4){return __wasi_##x(v1, v2, v3, v4);}

#define VEXPORT_1(x, t1) void x(t1 v1){return __wasi_##x(v1);}

EXPORT_2(fd_prestat_get, __wasi_fd_t, wasi_ptr_raw);
EXPORT_3(fd_prestat_dir_name, __wasi_fd_t, wasi_ptr_raw, wasi_size_t);
EXPORT_2(environ_sizes_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(environ_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(args_sizes_get, wasi_ptr_raw, wasi_ptr_raw);
EXPORT_2(args_get, wasi_ptr_raw, wasi_ptr_raw);
VEXPORT_1(proc_exit, __wasi_exitcode_t);
EXPORT_2(fd_fdstat_get, __wasi_fd_t, wasi_ptr_raw);
EXPORT_1(fd_close, __wasi_fd_t);
EXPORT_4(fd_seek, __wasi_fd_t, uint64_t, uint32_t, wasi_ptr_raw);
EXPORT_4(fd_write, __wasi_fd_t, wasi_ptr_raw, uint32_t, wasi_ptr_raw);

#endif // HOST

#endif // WASI_IMPL_HPP_INCLUDED
