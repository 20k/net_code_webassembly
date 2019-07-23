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
#include <set>

struct file_desc
{
    bool is_preopen = false;

    __wasi_fd_t fd = 0;
    std::string relative_path = ".";
    __wasi_rights_t fs_rights_base = 0;
    __wasi_rights_t fs_rights_inheriting = 0;
    __wasi_fdflags_t fs_flags = 0;
    __wasi_filetype_t fs_filetype = 0;
};

struct preopened
{
    std::map<__wasi_fd_t, file_desc> files;

    std::set<int> used_fds;

    int next_fd = 4;

    int get_next_fd()
    {
        used_fds.insert(next_fd);
        return next_fd++;
    }

    file_desc make_preopen(const std::string& path)
    {
        file_desc desc;
        desc.fd = 3;
        desc.relative_path = std::filesystem::path(path).string();
        desc.fs_rights_base =   __WASI_RIGHT_FD_DATASYNC |
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

        desc.fs_rights_inheriting = desc.fs_rights_base;

        desc.fs_flags = __WASI_FDFLAG_SYNC;
        desc.fs_filetype = __WASI_FILETYPE_DIRECTORY;
        desc.is_preopen = true;

        return desc;
    }

    preopened()
    {
        used_fds.insert(0);
        used_fds.insert(1);
        used_fds.insert(2);
        used_fds.insert(3);

        file_desc pre = make_preopen("./sandbox");

        files[pre.fd] = pre;
    }

    bool is_preopen(uint32_t fd)
    {
        assert(has_fd(fd));

        return files[fd].is_preopen;
    }

    bool has_fd(uint32_t fd)
    {
        return files.find(fd) != files.end();
    }

    std::string get_path(uint32_t fd)
    {
        assert(has_fd(fd));

        return files[fd].relative_path;
    }

    void close_fd(uint32_t fd)
    {
        assert(has_fd(fd));

        files.erase(files.find(fd));
    }
};

preopened file_sandbox;

__wasi_errno_t __wasi_clock_res_get(__wasi_clockid_t clock_id, wasi_ptr_t<__wasi_timestamp_t> resolution)
{
    if(clock_id == __WASI_CLOCK_MONOTONIC)
    {
        uint64_t res = std::chrono::steady_clock::period::num * 1000 * 1000 / std::chrono::steady_clock::period::den;

        *resolution = res;

        return __WASI_ESUCCESS;
    }

    if(clock_id == __WASI_CLOCK_REALTIME)
    {
        uint64_t res = std::chrono::system_clock::period::num * 1000 * 1000 / std::chrono::system_clock::period::den;

        *resolution = res;

        return __WASI_ESUCCESS;
    }

    #if 0
    ///ok this is wrong
    ///process cputime is literal cpu executed time, its a profiling thing it seems
    if(clock_id == __WASI_CLOCK_PROCESS_CPUTIME_ID)
    {
        uint64_t res = std::chrono::high_resolution_clock::period::num * 1000 * 1000 / std::chrono::high_resolution_clock::period::den;

        *resolution = res;

        return __WASI_ESUCCESS;
    }
    #endif // 0

    return __WASI_EINVAL;
}

__wasi_errno_t __wasi_clock_time_get(__wasi_clockid_t clock_id, __wasi_timestamp_t precision, wasi_ptr_t<__wasi_timestamp_t> ttme)
{
    if(clock_id == __WASI_CLOCK_MONOTONIC)
    {
        size_t ts_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

        *ttme = ts_nano;

        return __WASI_ESUCCESS;
    }

    if(clock_id == __WASI_CLOCK_REALTIME)
    {
        size_t ts_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        *ttme = ts_nano;

        return __WASI_ESUCCESS;
    }

    #if 0
    ///see clock_res_get
    if(clock_id == __WASI_CLOCK_PROCESS_CPUTIME_ID)
    {
        size_t ts_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        *ttme = ts_nano;

        return __WASI_ESUCCESS;
    }
    #endif // 0

    return __WASI_EINVAL;
}

__wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd, PTR(__wasi_prestat_t) buf)
{
    printf("Fd? %i\n", fd);

    if(file_sandbox.files.size() == 0)
        return __WASI_EBADF;

    if(file_sandbox.has_fd(fd) && file_sandbox.is_preopen(fd))
    {
        __wasi_prestat_t ret;
        ret.pr_type = __WASI_PREOPENTYPE_DIR;

        __wasi_prestat_t::__wasi_prestat_u::__wasi_prestat_u_dir_t dird;
        dird.pr_name_len = file_sandbox.files[fd].relative_path.length();

        ret.u.dir = dird;

        *buf = ret;

        return __WASI_ESUCCESS;
    }

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_fd_prestat_dir_name(__wasi_fd_t fd, PTR(char) path, wasi_size_t path_len)
{
    printf("FdDirName %i %i\n", fd, path_len);

    if(file_sandbox.has_fd(fd) && file_sandbox.is_preopen(fd))
    {
        std::string cname = file_sandbox.files[fd].relative_path;

        for(size_t i=0; i < cname.size() && i < path_len; i++)
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

    for(size_t i=0; i < strlen(name); i++)
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

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    printf("Yaystat\n");

    const file_desc& desc = file_sandbox.files[fd];

    __wasi_fdstat_t preval;

    preval.fs_filetype = desc.fs_filetype;
    preval.fs_flags = desc.fs_flags;
    preval.fs_rights_base = desc.fs_rights_base;
    preval.fs_rights_inheriting = desc.fs_rights_inheriting;

    *buf = preval;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd)
{
    printf("Close\n");

    if(fd == 0 || fd == 1 || fd == 2)
        return __WASI_EBADF;

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    file_sandbox.close_fd(fd);

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

        for(size_t i=0; i < iovs_len; i++)
        {
            __wasi_ciovec_t single = iovs[i];

            const wasi_ptr_t<void> buf = single.buf;

            wasi_ptr_t<char> tchr(0);
            tchr.val = buf.val;

            for(size_t kk=0; kk < single.buf_len; kk++)
            {
                func(tchr[kk]);
            }

            written += single.buf_len;
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

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;


    return __WASI_EBADF;
}

#endif // WASI_IMPL_HPP_INCLUDED
