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

        assert(val < mem_0.size());
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
        assert(val + idx * sizeof(T) >= 0);

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
#include <thread>

#define NO_OLDNAMES

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct file_desc
{
    bool is_preopen = false;

    ///so. as far as I can tell, std::filesystem doesn't really do locking
    ///like, you can't "open" a file which is a right shame
    __wasi_fd_t fd = 0;
    int64_t portable_fd = 0;
    std::string relative_path = ".";
    __wasi_rights_t fs_rights_base = 0;
    __wasi_rights_t fs_rights_inheriting = 0;
    __wasi_fdflags_t fs_flags = 0;
    __wasi_filetype_t fs_filetype = 0;
};

#include <climits>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define stat __stat64
#define fstat _fstat64
#define fileno _fileno

#define	_S_IFBLK	0x3000

#define S_IFMT      _S_IFMT
#define S_IFDIR     _S_IFDIR
#define S_IFCHR     _S_IFCHR
#define S_IFREG     _S_IFREG
#define S_IREAD     _S_IREAD
#define S_IWRITE    _S_IWRITE
#define S_IEXEC     _S_IEXEC
#define	S_IFIFO		_S_IFIFO
#define	S_IFBLK		_S_IFBLK

#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)

#define open _open
#define close _close

#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_RDWR _O_RDWR
#define O_APPEND _O_APPEND
#define O_CREAT _O_CREAT
#define O_TRUNC _O_TRUNC
#define O_EXCL _O_EXCL
#define O_TEXT _O_TEXT
#define O_BINARY _O_BINARY
#define O_RAW _O_BINARY
#define O_TEMPORARY _O_TEMPORARY
#define O_NOINHERIT _O_NOINHERIT
#define O_SEQUENTIAL _O_SEQUENTIAL
#define O_RANDOM _O_RANDOM
#define O_ACCMODE _O_ACCMODE

__wasi_errno_t get_read_fd_wrapper(const std::string& path, file_desc& out, __wasi_oflags_t open_flags)
{
    DWORD dwCreationDisposition = OPEN_EXISTING;

    if((open_flags & __WASI_O_CREAT) > 0)
        dwCreationDisposition = CREATE_ALWAYS;

    if((open_flags & __WASI_O_EXCL) > 0)
        dwCreationDisposition = CREATE_NEW;

    if((open_flags & __WASI_O_TRUNC) > 0)
        dwCreationDisposition = TRUNCATE_EXISTING;

    bool emulate_truncate = false;

    if((open_flags & __WASI_O_TRUNC) > 0 && (open_flags & __WASI_O_CREAT) > 0)
    {
        emulate_truncate = true;
    }

    if(emulate_truncate)
    {
        if((open_flags & __WASI_O_EXCL) == 0)
            dwCreationDisposition = CREATE_ALWAYS;
        else
            dwCreationDisposition = CREATE_NEW;
    }

    std::cout << "CREAT " << dwCreationDisposition << " NAME? " << path << std::endl;

    HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, dwCreationDisposition, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    printf("H2\n");

    if(hFile == INVALID_HANDLE_VALUE)
        return __WASI_EACCES;

    BY_HANDLE_FILE_INFORMATION fhandle = {};
    GetFileInformationByHandle(hFile, &fhandle);

    int nHandle = _open_osfhandle((intptr_t)hFile, 0);

    if(nHandle == -1)
    {
        ::CloseHandle(hFile);
        return __WASI_EACCES;
    }

    out.portable_fd = nHandle;
    out.fs_filetype = __WASI_FILETYPE_REGULAR_FILE;

    if((fhandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
        out.fs_filetype = __WASI_FILETYPE_DIRECTORY;

    /*{
        struct stat st;
        fstat(out.portable_fd, &st);

        std::cout << "FMODE " << std::to_string(st.st_mode) << std::endl;

        if((st.st_mode & S_IFDIR) > 0)
        {
            std::cout << "IS DIR\n";
        }
    }*/

    if(out.fs_filetype != __WASI_FILETYPE_DIRECTORY && ((open_flags & __WASI_O_DIRECTORY) > 0))
    {
        close(nHandle);

        return __WASI_ENOENT;
    }

    if(emulate_truncate)
    {
        errno_t err = _chsize_s(nHandle, 0);

        if(err == EACCES)
            return __WASI_EACCES;
        if(err == EBADF)
            return __WASI_EBADF;
        if(err == ENOSPC)
            return __WASI_ENOSPC;
        if(err == EINVAL)
            return __WASI_EINVAL;

        if(err != 0)
            return __WASI_ENOTCAPABLE;
    }

    return __WASI_ESUCCESS;
}

#define write _write
#define read _read

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

__wasi_errno_t get_read_fd_wrapper(const std::string& path, file_desc& out, __wasi_oflags_t open_flags)
{
    int flags = 0;

    if((open_flags & __WASI_O_CREAT) > 0)
        flags |= O_CREAT;

    if((open_flags & __WASI_O_EXCL) > 0)
        flags |= O_EXCL;

    if((open_flags & __WASI_O_TRUNC) > 0)
        flags |= O_TRUNC;

    if((open_flags & __WASI_O_DIRECTORY) > 0)
        flags |= O_DIRECTORY;

    out.portable_fd = open(path.c_str(), flags);

    if(out.portable_fd == -1)
        return __WASI_EACCES;

    struct stat st;
    fstat(out.portable_fd, &st);

    out.fs_filetype = __WASI_FILETYPE_UNKNOWN;

    if(S_ISREG(st.st_mode))
        out.fs_filetype = __WASI_FILETYPE_REGULAR_FILE;
    if(S_ISDIR(st.st_mode))
        out.fs_filetype = __WASI_FILETYPE_DIRECTORY;
    if(S_ISFIFO(st.st_mode))
        out.fs_filetype = __WASI_FILETYPE_UNKNOWN;
    if(S_ISCHR(st.st_mode))
        out.fs_filetype = __WASI_FILETYPE_CHARACTER_DEVICE;
    if(S_ISBLK(st.st_mode))
        out.fs_filetype = __WASI_FILETYPE_BLOCK_DEVICE;

    return __WASI_ESUCCESS;
}
#endif // _WIN32

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
        desc.is_preopen = true;

        __wasi_errno_t err = get_read_fd_wrapper(path.c_str(), desc, 0);

        assert(err == __WASI_ESUCCESS);

        assert(desc.portable_fd != -1);
        assert(desc.fs_filetype == __WASI_FILETYPE_DIRECTORY);

        return desc;
    }

    __wasi_errno_t make_file(__wasi_fd_t base, const std::string& path, file_desc& out, __wasi_oflags_t open_flags)
    {
        if(!has_fd(base))
            return __WASI_EBADF;

        if(files[base].fs_filetype != __WASI_FILETYPE_DIRECTORY)
            return __WASI_ENOTDIR;

        std::filesystem::path theirs = path;
        std::filesystem::path mine = files[base].relative_path;

        std::filesystem::path their_requested_full = mine/theirs;

        std::filesystem::path rel = their_requested_full.std::filesystem::path::lexically_relative(std::filesystem::path(files[3].relative_path));

        std::filesystem::path fin = std::filesystem::path(their_requested_full).lexically_normal();

        std::string relative_path = rel.string();

        if(relative_path.size() > 0 && relative_path[0] == '.' && relative_path != ".")
            return __WASI_EACCES;

        ///should be sandbox/something
        std::string final_path = fin.string();

        file_desc desc;
        desc.relative_path = final_path;
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
        desc.is_preopen = false;

        desc.fd = get_next_fd();

        std::cout << "Flags? " << open_flags << std::endl;

        __wasi_errno_t err = get_read_fd_wrapper(final_path, desc, open_flags);

        if(err != __WASI_ESUCCESS)
            return err;

        out = desc;

        files[out.fd] = desc;

        return __WASI_ESUCCESS;
    }

    preopened()
    {
        used_fds.insert(0);
        used_fds.insert(1);
        used_fds.insert(2);
        used_fds.insert(3);

        file_desc pre = make_preopen("sandbox");

        files[pre.fd] = pre;
    }

    ~preopened()
    {
        for(auto& i : files)
        {
            close(i.second.portable_fd);
        }
    }

    bool is_preopen(uint32_t fd)
    {
        assert(has_fd(fd));

        return files[fd].is_preopen;
    }

    bool has_fd(uint32_t fd) const
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

        close(files[fd].portable_fd);

        files.erase(files.find(fd));
    }

    bool can_fd(uint32_t fd, __wasi_rights_t right)
    {
        assert(has_fd(fd));

        return (files[fd].fs_rights_base & right) > 0;
    }

    void set_flags(uint32_t fd, __wasi_fdflags_t flags)
    {
        assert(has_fd(fd));

        files[fd].fs_flags = flags;
    }

    void datasync(uint32_t fd)
    {
        assert(has_fd(fd));
    }

    ///TODO:
    size_t get_size_fd(uint32_t fd)
    {
        assert(has_fd(fd));

        return 0;
    }

    ///TODO:
    bool resize_fd(uint32_t fd, size_t new_size)
    {
        assert(has_fd(fd));

        ///CHECK SANDBOX SIZE
        ///CHECK DISK SPACE

        return false;
    }

    __wasi_errno_t write_fd(uint32_t fd, wasi_ptr_t<char> data, wasi_size_t len, size_t& out_bytes)
    {
        assert(has_fd(fd));

        if(len <= 0)
            return __WASI_ESUCCESS;

        data[0];
        data[len-1];

        size_t processed = 0;

        file_desc& desc = files[fd];

        if(desc.fs_filetype != __WASI_FILETYPE_REGULAR_FILE)
            return __WASI_EBADF;

        while(processed < len)
        {
            int next = write(desc.portable_fd, &data[processed], len - processed);

            if(next == -1)
                return __WASI_EBADF;

            processed += next;
            out_bytes = processed;
        }

        return __WASI_ESUCCESS;
    }

    __wasi_errno_t read_fd(uint32_t fd, wasi_ptr_t<char> data, wasi_size_t len, size_t& out_bytes)
    {
        assert(has_fd(fd));

        if(len <= 0)
            return __WASI_ESUCCESS;

        data[0];
        data[len-1];

        size_t processed = 0;

        file_desc& desc = files[fd];

        if(desc.fs_filetype != __WASI_FILETYPE_REGULAR_FILE)
            return __WASI_EBADF;

        while(processed < len)
        {
            int next = read(desc.portable_fd, &data[processed], len - processed);

            if(next == -1)
                return __WASI_EBADF;

            processed += next;
            out_bytes = processed;
        }

        return __WASI_ESUCCESS;
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
        dird.pr_name_len = file_sandbox.files[fd].relative_path.length() + 1;

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

        path[path_len-1] = 0;

        return __WASI_ESUCCESS;
    }

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_fd_advise(__wasi_fd_t fd, __wasi_filesize_t offset, __wasi_filesize_t len, __wasi_advice_t advice)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_ADVISE))
        return __WASI_ENOTCAPABLE;

    return __WASI_ESUCCESS;
}

//__wasi_errno_t __wasi_fd_allocate()

__wasi_errno_t __wasi_fd_datasync(__wasi_fd_t fd)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_DATASYNC))
        return __WASI_ENOTCAPABLE;

    file_sandbox.datasync(fd);
    return __WASI_ESUCCESS;
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

__wasi_errno_t __wasi_fd_fdstat_set_flags(__wasi_fd_t fd, __wasi_fdflags_t flags)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_FDSTAT_SET_FLAGS))
        return __WASI_ENOTCAPABLE;

    file_sandbox.set_flags(fd, flags);

    return __WASI_ESUCCESS;
}

#define CLEARBIT(x, y) x &= ~(1ULL << (uint64_t)y)

__wasi_errno_t __wasi_fd_fdstat_set_rights(__wasi_fd_t fd, __wasi_rights_t fs_rights_base, __wasi_rights_t fs_rights_inheriting)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    const file_desc& my_desc_c = file_sandbox.files[fd];

    for(int i=0; i < (int)sizeof(__wasi_rights_t) * 8; i++)
    {
        int their_requested_base = (fs_rights_base >> i) & 1;
        int their_requested_inherit = (fs_rights_inheriting >> i) & 1;

        int my_cur_base = (my_desc_c.fs_rights_base >> i) & 1;
        int my_cur_inherit = (my_desc_c.fs_rights_inheriting >> i) & 1;

        if(my_cur_base == 0 && their_requested_base == 1)
            return __WASI_ENOTCAPABLE;

        if(my_cur_inherit == 0 && their_requested_inherit == 1)
            return __WASI_ENOTCAPABLE;
    }

    file_desc& my_desc = file_sandbox.files[fd];

    for(int i=0; i < (int)sizeof(__wasi_rights_t) * 8; i++)
    {
        int their_requested_base = (fs_rights_base >> i) & 1;
        int their_requested_inherit = (fs_rights_inheriting >> i) & 1;

        ///can only be used to strip rights
        if(their_requested_base == 0)
        {
            CLEARBIT(my_desc.fs_rights_base, i);
        }

        if(their_requested_inherit == 0)
        {
            CLEARBIT(my_desc.fs_rights_inheriting, i);
        }
    }

    return __WASI_ESUCCESS;
}

#undef CLEARBIT

__wasi_errno_t __wasi_fd_filestat_get(__wasi_fd_t fd, wasi_ptr_t<__wasi_filestat_t> buf)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_FILESTAT_GET))
        return __WASI_ENOTCAPABLE;

    const file_desc& fle = file_sandbox.files[fd];

    __wasi_filestat_t ret;
    ret.st_dev = 0;
    ret.st_ino = 0; ///ERROR NEED TO DO INODES
    ret.st_filetype = fle.fs_filetype;
    ret.st_nlink = 0;
    ret.st_size = file_sandbox.get_size_fd(fd);
    ret.st_atim = 0; ///TODO: ATIM
    ret.st_mtim = 0; ///TODO: MTIM
    ret.st_ctim = 0; ///TODO: CTIM

    *buf = ret;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_filestat_set_size(__wasi_fd_t fd, __wasi_filesize_t st_size)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_FILESTAT_SET_SIZE))
        return __WASI_ENOTCAPABLE;

    if(!file_sandbox.resize_fd(fd, st_size))
        return __WASI_EFBIG;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_filestat_set_times(__wasi_fd_t fd, __wasi_timestamp_t st_atim, __wasi_timestamp_t st_mtim, __wasi_fstflags_t fstflags)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_FILESTAT_SET_TIMES))
        return __WASI_ENOTCAPABLE;

    return __WASI_ENOTCAPABLE;
    //return __WASI_ESUCCESS;
}

//fd_pread
//fd_pwrite
//fd_read
//fd_write
//fd_readdir
//fd_renumber
//fd_seek
//fd_sync
//fd_tell
//fd_write - update impl
//path_create_directory
//path_filestat_get
//path_filestat_set_times
//path_link
//path_open
//path_readlink
//path_remove_directory
//path_rename
//path_symlink
//path_unlink_file
//poll_oneoff
//sock_recv
//sock_send
//sock_shutdown

///implement path_open first

#include <random>

uint8_t get_next_random_byte()
{
    static std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char> eng;

    return eng();
}

#define HALT_ON(x) if(sig == x) assert(false);

__wasi_errno_t __wasi_proc_raise(__wasi_signal_t sig)
{
    HALT_ON(__WASI_SIGABRT);
    HALT_ON(__WASI_SIGALRM); ///is this really necessary in the spec
    HALT_ON(__WASI_SIGBUS);
    HALT_ON(__WASI_SIGCONT);
    HALT_ON(__WASI_SIGFPE);
    HALT_ON(__WASI_SIGHUP);
    HALT_ON(__WASI_SIGILL);
    HALT_ON(__WASI_SIGINT);
    HALT_ON(__WASI_SIGKILL);
    HALT_ON(__WASI_SIGPIPE);
    HALT_ON(__WASI_SIGQUIT);
    HALT_ON(__WASI_SIGSEGV);
    HALT_ON(__WASI_SIGSTOP);
    HALT_ON(__WASI_SIGSYS);
    HALT_ON(__WASI_SIGTERM);
    HALT_ON(__WASI_SIGTRAP);
    HALT_ON(__WASI_SIGTSTP);
    HALT_ON(__WASI_SIGTTIN);
    HALT_ON(__WASI_SIGTTOU);
    HALT_ON(__WASI_SIGUSR1);
    HALT_ON(__WASI_SIGUSR2);
    HALT_ON(__WASI_SIGVTALRM);
    HALT_ON(__WASI_SIGXCPU);
    HALT_ON(__WASI_SIGXFSZ);

    if(sig == __WASI_SIGCHLD)
    {
        return __WASI_ESUCCESS;
    }
    if(sig == __WASI_SIGCONT)
    {
        ///do nothing because i'm not sure there's a way for signals
        ///to actually be supplied asynchronously
        return __WASI_ESUCCESS;
    }
    if(sig == __WASI_SIGURG)
    {
        return __WASI_ESUCCESS;
    }

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_random_get(wasi_ptr_t<void> buf, uint32_t buf_len)
{
    wasi_ptr_t<char> cbuf(0);
    cbuf.val = buf.val;

    for(uint64_t i=0; i < (uint64_t)buf_len; i++)
    {
        cbuf[i] = get_next_random_byte();
    }

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_sched_yield()
{
    std::this_thread::yield();
    return __WASI_ESUCCESS;
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

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd)
{
    printf("Close\n");

    if(fd == 0 || fd == 1 || fd == 2)
        return __WASI_EBADF;

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(file_sandbox.is_preopen(fd))
        return __WASI_EBADF;

    file_sandbox.close_fd(fd);

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, PTR(__wasi_filesize_t) newoffset)
{
    printf("Seek\n");

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_SEEK))
        return __WASI_ENOTCAPABLE;

    return __WASI_EBADF;
}

__wasi_errno_t __wasi_path_open(__wasi_fd_t dirfd,
                                __wasi_lookupflags_t dirflags,
                                const wasi_ptr_t<char>path,
                                wasi_size_t path_len,
                                __wasi_oflags_t oflags,
                                __wasi_rights_t fs_rights_base,
                                __wasi_rights_t fs_rights_inheriting,
                                __wasi_fdflags_t fs_flags,
                                wasi_ptr_t<__wasi_fd_t> fd)
{
    assert(file_sandbox.has_fd(dirfd));

    printf("Openasdfasdf\n");

    if(path_len == 0)
        return __WASI_EACCES;

    std::string pth;

    for(size_t i=0; i < path_len; i++)
    {
        pth += std::string(1, path[i]);
    }

    file_desc out;
    __wasi_errno_t err = file_sandbox.make_file(dirfd, pth, out, oflags);

    if(err != __WASI_ESUCCESS)
        return err;

    *fd = out.fd;

    return __WASI_ESUCCESS;
}


__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd, const wasi_ptr_t<__wasi_iovec_t> iovs, wasi_size_t iovs_len, wasi_ptr_t<uint32_t> nread)
{
    printf("Read %i\n", fd);

    ///stdin
    if(fd == 0)
    {

    }

    ///stdout
    if(fd == 1)
        return __WASI_EBADF;

    ///stderr
    if(fd == 2)
        return __WASI_EBADF;

    *nread = 0;

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    for(size_t i=0; i < iovs_len; i++)
    {
        __wasi_iovec_t single = iovs[i];

        const wasi_ptr_t<void> buf = single.buf;

        wasi_ptr_t<char> tchr(0);
        tchr.val = buf.val;

        size_t out_bytes = 0;
        __wasi_errno_t err = file_sandbox.read_fd(fd, tchr, single.buf_len, out_bytes);

        if(err != __WASI_ESUCCESS)
            return err;

        *nread += out_bytes;
    }

    return __WASI_ESUCCESS;
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
        return __WASI_EBADF;
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

    for(size_t i=0; i < iovs_len; i++)
    {
        __wasi_ciovec_t single = iovs[i];

        const wasi_ptr_t<void> buf = single.buf;

        wasi_ptr_t<char> tchr(0);
        tchr.val = buf.val;

        size_t out_bytes = 0;
        __wasi_errno_t err = file_sandbox.write_fd(fd, tchr, single.buf_len, out_bytes);

        if(err != __WASI_ESUCCESS)
            return err;

        *nwritten += out_bytes;
    }

    return __WASI_ESUCCESS;
}

#endif // WASI_IMPL_HPP_INCLUDED
