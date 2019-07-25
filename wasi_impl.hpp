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

std::string make_str(const wasi_ptr_t<char> ptr, wasi_size_t len)
{
    if(len == 0)
        return "";

    ptr[0];
    ptr[len-1];

    return std::string(&ptr[0], len);
}

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

#define WERR(x) if(x == err) return __WASI_##x;

#include <errno.h>

__wasi_errno_t errno_to_wasi(int err)
{
    if(err == 0)
        return __WASI_ESUCCESS;

    WERR(E2BIG);
    WERR(EACCES);
    WERR(EADDRINUSE);
    WERR(EADDRNOTAVAIL);
    WERR(EAFNOSUPPORT);
    WERR(EAGAIN);
    WERR(EALREADY);
    WERR(EBADF);
    WERR(EBADMSG);
    WERR(EBUSY);
    WERR(ECANCELED);
    WERR(ECHILD);
    WERR(ECONNABORTED);
    WERR(ECONNREFUSED);
    WERR(ECONNRESET);
    WERR(EDEADLK);
    WERR(EDESTADDRREQ);
    WERR(EDOM);
    //WERR(EDQUOT);
    WERR(EEXIST);
    WERR(EFAULT);
    WERR(EFBIG);
    WERR(EHOSTUNREACH);
    WERR(EIDRM);
    WERR(EILSEQ);
    WERR(EINPROGRESS);
    WERR(EINTR);
    WERR(EINVAL);
    WERR(EIO);
    WERR(EISCONN);
    WERR(EISDIR);
    WERR(ELOOP);
    WERR(EMFILE);
    WERR(EMLINK);
    WERR(EMSGSIZE);
    //WERR(EMULTIHOP);
    WERR(ENAMETOOLONG);
    WERR(ENETDOWN);
    WERR(ENETRESET);
    WERR(ENETUNREACH);
    WERR(ENFILE);
    WERR(ENOBUFS);
    WERR(ENODEV);
    WERR(ENOENT);
    WERR(ENOEXEC);
    WERR(ENOLCK);
    WERR(ENOLINK);
    WERR(ENOMEM);
    WERR(ENOMSG);
    WERR(ENOPROTOOPT);
    WERR(ENOSPC);
    WERR(ENOSYS);
    WERR(ENOTCONN);
    WERR(ENOTDIR);
    WERR(ENOTEMPTY);
    WERR(ENOTRECOVERABLE);
    WERR(ENOTSOCK);
    WERR(ENOTSUP);
    WERR(ENOTTY);
    WERR(ENXIO);
    WERR(EOVERFLOW);
    WERR(EOWNERDEAD);
    WERR(EPERM);
    WERR(EPIPE);
    WERR(EPROTO);
    WERR(EPROTONOSUPPORT);
    WERR(EPROTOTYPE);
    WERR(ERANGE);
    WERR(EROFS);
    WERR(ESPIPE);
    WERR(ESRCH);
    //WERR(ESTALE);
    WERR(ETIMEDOUT);
    WERR(ETXTBSY);
    WERR(EXDEV);

    return __WASI_ENOTCAPABLE;
}

#define WASI_ERRNO() errno_to_wasi(errno)

intptr_t get_std_inouterr_platform_handle(int std_fd);

///actually working fstat info
struct cfstat_info
{
    __wasi_filetype_t type = __WASI_FILETYPE_UNKNOWN;
};

__wasi_errno_t cfstat(int64_t fd, cfstat_info* buf);

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
#define ftruncate _chsize
#define fdatasync _commit
#define fsync _commit
#define unlink _unlink
#define lseek _lseeki64

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

///handle based rename is SetFileInformationByHandle

intptr_t get_std_inouterr_platform_handle(int which)
{
    assert(which == 0 || which == 1 || which == 2);

    if(which == 0)
    {
        return (intptr_t)GetStdHandle(STD_INPUT_HANDLE);
    }

    if(which == 1)
    {
        return (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if(which == 2)
    {
        return (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
    }

    assert(false);
    return -1;
}

__wasi_errno_t cfstat(int64_t fd, cfstat_info* buf)
{
    assert(buf != nullptr);

    if(fd < 0)
        return __WASI_EBADF;

    intptr_t handle = 0;

    if(fd == 0 || fd == 1 || fd == 2)
    {
        handle = get_std_inouterr_platform_handle(fd);

        if(handle == -1)
            return __WASI_EBADF;
    }
    else
    {
        handle = _get_osfhandle(fd);

        if(handle == -1)
            return WASI_ERRNO();

        if(handle == -2)
            return __WASI_EBADF;
    }

    buf->type = __WASI_FILETYPE_UNKNOWN;

    DWORD fret = GetFileType((HANDLE)handle);

    if(fret == FILE_TYPE_CHAR)
        buf->type = __WASI_FILETYPE_CHARACTER_DEVICE;

    ///pipe = socket or anon/named pipe
    if(fret == FILE_TYPE_PIPE)
    {
        buf->type = __WASI_FILETYPE_CHARACTER_DEVICE;
    }

    if(fret == FILE_TYPE_UNKNOWN)
    {
        buf->type = FILE_TYPE_UNKNOWN;

        if(GetLastError() != NO_ERROR)
            return WASI_ERRNO();
    }

    if(fret == FILE_TYPE_DISK)
    {
        BY_HANDLE_FILE_INFORMATION fhandle = {};
        GetFileInformationByHandle((HANDLE)handle, &fhandle);

        buf->type = __WASI_FILETYPE_REGULAR_FILE;

        if((fhandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
            buf->type = __WASI_FILETYPE_DIRECTORY;
    }

    return __WASI_ESUCCESS;
}

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

    int share_flags = 0;

    if((open_flags & __WASI_O_DIRECTORY) > 0)
    {
        share_flags = FILE_SHARE_WRITE | FILE_SHARE_READ;
    }

    std::cout << "CREAT " << dwCreationDisposition << " NAME? " << path << std::endl;

    HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, share_flags, nullptr, dwCreationDisposition, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if(hFile == INVALID_HANDLE_VALUE)
        return WASI_ERRNO();

    int nHandle = _open_osfhandle((intptr_t)hFile, 0);

    if(nHandle == -1)
    {
        __wasi_errno_t err = WASI_ERRNO();

        ::CloseHandle(hFile);
        return err;
    }

    out.portable_fd = nHandle;

    cfstat_info buf;
    cfstat(out.portable_fd, &buf);

    out.fs_filetype = buf.type;

    #if 0
    {
        intptr_t hndl = _get_osfhandle(nHandle);

        BY_HANDLE_FILE_INFORMATION fhandle2 = {};
        GetFileInformationByHandle((HANDLE)hndl, &fhandle2);

        struct stat st;
        fstat(nHandle, &st);

        std::cout << "is dir through winapi " << ((fhandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0) << std::endl;
        std::cout << "is dir through cstyle api " << S_ISDIR(st.st_mode) << std::endl;
    }
    #endif // 0

    if(out.fs_filetype != __WASI_FILETYPE_DIRECTORY && ((open_flags & __WASI_O_DIRECTORY) > 0))
    {
        close(nHandle);

        return __WASI_ENOTDIR;
    }

    if(emulate_truncate)
    {
        errno_t err = _chsize_s(nHandle, 0);

        if(err)
            return errno_to_wasi(err);
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

intptr_t get_std_inouterr_platform_handle(int which)
{
    assert(which == 0 || which == 1 || which == 2);

    return which;
}

__wasi_errno_t cfstat(int64_t fd, cfstat_info* buf)
{
    assert(buf);

    struct stat st;
    int rval = fstat(fd, &st);

    if(rval != 0)
        return WASI_ERRNO();

    buf->type = __WASI_FILETYPE_UNKNOWN;

    if(S_ISREG(st.st_mode))
        buf->type = __WASI_FILETYPE_REGULAR_FILE;
    if(S_ISDIR(st.st_mode))
        buf->type = __WASI_FILETYPE_DIRECTORY;
    if(S_ISFIFO(st.st_mode))
        buf->type = __WASI_FILETYPE_CHARACTER_DEVICE;
    if(S_ISCHR(st.st_mode))
        buf->type = __WASI_FILETYPE_CHARACTER_DEVICE;
    if(S_ISBLK(st.st_mode))
        buf->type = __WASI_FILETYPE_BLOCK_DEVICE;

    return __WASI_ESUCCESS;
}

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
        return WASI_ERRNO();

    cfstat_info buf;
    cfstat(out.portable_fd, &buf);

    out.fs_filetype = buf.type;

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
                                __WASI_RIGHT_PATH_LINK_SOURCE |
                                __WASI_RIGHT_PATH_LINK_TARGET |
                                __WASI_RIGHT_PATH_OPEN |
                                __WASI_RIGHT_FD_READDIR |
                                __WASI_RIGHT_PATH_READLINK |
                                __WASI_RIGHT_PATH_RENAME_SOURCE |
                                __WASI_RIGHT_PATH_RENAME_TARGET |
                                __WASI_RIGHT_PATH_FILESTAT_GET |
                                __WASI_RIGHT_PATH_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_PATH_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_FD_FILESTAT_GET |
                                __WASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_FD_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_PATH_SYMLINK |
                                __WASI_RIGHT_PATH_REMOVE_DIRECTORY |
                                __WASI_RIGHT_PATH_UNLINK_FILE |
                                __WASI_RIGHT_POLL_FD_READWRITE |
                                __WASI_RIGHT_SOCK_SHUTDOWN;

        desc.fs_rights_inheriting = desc.fs_rights_base;

        desc.fs_flags = __WASI_FDFLAG_SYNC;
        desc.is_preopen = true;

        __wasi_errno_t err = get_read_fd_wrapper(path.c_str(), desc, __WASI_O_DIRECTORY);

        assert(err == __WASI_ESUCCESS);

        assert(desc.portable_fd != -1);
        assert(desc.fs_filetype == __WASI_FILETYPE_DIRECTORY);

        return desc;
    }

    bool path_in_sandbox(const std::filesystem::path& path)
    {
        std::filesystem::path rel = path.std::filesystem::path::lexically_relative(std::filesystem::path(files[3].relative_path));

        std::string relative_path = rel.string();

        if(relative_path.size() > 0 && relative_path[0] == '.' && relative_path != ".")
            return false;

        return true;
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

        std::filesystem::path fin = std::filesystem::path(their_requested_full).lexically_normal();

        if(!path_in_sandbox(their_requested_full))
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
                                __WASI_RIGHT_PATH_LINK_SOURCE |
                                __WASI_RIGHT_PATH_LINK_TARGET |
                                __WASI_RIGHT_PATH_OPEN |
                                __WASI_RIGHT_FD_READDIR |
                                __WASI_RIGHT_PATH_READLINK |
                                __WASI_RIGHT_PATH_RENAME_SOURCE |
                                __WASI_RIGHT_PATH_RENAME_TARGET |
                                __WASI_RIGHT_PATH_FILESTAT_GET |
                                __WASI_RIGHT_PATH_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_PATH_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_FD_FILESTAT_GET |
                                __WASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                __WASI_RIGHT_FD_FILESTAT_SET_TIMES |
                                __WASI_RIGHT_PATH_SYMLINK |
                                __WASI_RIGHT_PATH_REMOVE_DIRECTORY |
                                __WASI_RIGHT_PATH_UNLINK_FILE |
                                __WASI_RIGHT_POLL_FD_READWRITE |
                                __WASI_RIGHT_SOCK_SHUTDOWN;

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

        {
            file_desc std_fs;
            std_fs.portable_fd = 0;
            std_fs.fd = 0;
            std_fs.relative_path = "";
            std_fs.fs_rights_base = __WASI_RIGHT_FD_READ;
            std_fs.fs_rights_inheriting = std_fs.fs_rights_base;
            std_fs.fs_flags = __WASI_FDFLAG_SYNC;

            cfstat_info st;
            cfstat(std_fs.portable_fd, &st);

            std_fs.fs_filetype = st.type;
            files[std_fs.fd] = std_fs;
        }

        {
            file_desc std_fs;
            std_fs.portable_fd = 1;
            std_fs.fd = 1;
            std_fs.relative_path = "";
            std_fs.fs_rights_base = __WASI_RIGHT_FD_WRITE;
            std_fs.fs_rights_inheriting = std_fs.fs_rights_base;
            std_fs.fs_flags = __WASI_FDFLAG_SYNC;

            cfstat_info st;
            cfstat(std_fs.portable_fd, &st);

            std_fs.fs_filetype = st.type;
            files[std_fs.fd] = std_fs;
        }

        {
            file_desc std_fs;
            std_fs.portable_fd = 2;
            std_fs.fd = 2;
            std_fs.relative_path = "";
            std_fs.fs_rights_base = __WASI_RIGHT_FD_WRITE;
            std_fs.fs_rights_inheriting = std_fs.fs_rights_base;
            std_fs.fs_flags = __WASI_FDFLAG_SYNC;

            cfstat_info st;
            cfstat(std_fs.portable_fd, &st);

            std_fs.fs_filetype = st.type;
            files[std_fs.fd] = std_fs;
        }

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

    __wasi_errno_t datasync(uint32_t fd)
    {
        assert(has_fd(fd));

        int res = fdatasync(files[fd].portable_fd);

        if(res != 0)
            return WASI_ERRNO();

        return __WASI_ESUCCESS;
    }

    __wasi_errno_t sync(uint32_t fd)
    {
        assert(has_fd(fd));

        int res = fsync(files[fd].portable_fd);

        if(res != 0)
            return WASI_ERRNO();

        return __WASI_ESUCCESS;
    }

    ///TODO:
    size_t get_size_fd(uint32_t fd)
    {
        assert(has_fd(fd));

        return 0;
    }

    ///TODO:
    __wasi_errno_t resize_fd(uint32_t fd, size_t new_size)
    {
        assert(has_fd(fd));

        ///CHECK SANDBOX SIZE
        ///CHECK DISK SPACE

        ///ftruncate64?
        int success = ftruncate(files[fd].portable_fd, new_size);

        if(success != 0)
            return WASI_ERRNO();

        return __WASI_ESUCCESS;
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

        if(desc.fs_filetype != __WASI_FILETYPE_REGULAR_FILE && desc.fs_filetype != __WASI_FILETYPE_CHARACTER_DEVICE)
            return __WASI_EBADF;

        ///so. I think it might not be my responsibility to do this
        //while(processed < len)
        {
            int next = write(desc.portable_fd, &data[processed], len - processed);

            if(next == -1)
                return WASI_ERRNO();

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

        if(desc.fs_filetype != __WASI_FILETYPE_REGULAR_FILE && desc.fs_filetype != __WASI_FILETYPE_CHARACTER_DEVICE)
            return __WASI_EBADF;

        //while(processed < len)
        {
            int next = read(desc.portable_fd, &data[processed], len - processed);

            if(next == -1)
                return WASI_ERRNO();

            processed += next;
            out_bytes = processed;
        }

        return __WASI_ESUCCESS;
    }
};

preopened file_sandbox;

///TODO: Implement the other clocks
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

///TODO: Do I need to implement this?
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

    __wasi_errno_t err = file_sandbox.datasync(fd);

    return err;
}

__wasi_errno_t __wasi_fd_sync(__wasi_fd_t fd)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_SYNC))
        return __WASI_ENOTCAPABLE;

    __wasi_errno_t err = file_sandbox.sync(fd);

    return err;
}

///TODO: Mostly fine but might want to stat real file to check for external influences
__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, PTR(__wasi_fdstat_t) buf)
{
    printf("FdStat %i\n", fd);

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

///TODO: Do I actually need to do anything here?
///I suppose I at least need to implement append and sync, switching
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

///TODO: Clean this up
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

    __wasi_errno_t err = file_sandbox.resize_fd(fd, st_size);

    if(err != __WASI_ESUCCESS)
        return err;

    return __WASI_ESUCCESS;
}

///TODO: This
__wasi_errno_t __wasi_fd_filestat_set_times(__wasi_fd_t fd, __wasi_timestamp_t st_atim, __wasi_timestamp_t st_mtim, __wasi_fstflags_t fstflags)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_FILESTAT_SET_TIMES))
        return __WASI_ENOTCAPABLE;

    return __WASI_ENOTCAPABLE;
    //return __WASI_ESUCCESS;
}

///0/1/2 fds need to be merged into fd_write and fd_read

//fd_pread
//fd_pwrite
//fd_readdir
//fd_renumber
//path_create_directory
//path_filestat_get
//path_filestat_set_times
//path_link
//path_open
//path_readlink
//path_symlink
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
    size_t len = strlen(name);

    for(size_t i=0; i < len; i++)
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

    int64_t off = lseek(fd, offset, whence);

    if(off == -1)
        return WASI_ERRNO();

    *newoffset = off;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_tell(__wasi_fd_t fd, wasi_ptr_t<__wasi_filesize_t> newoffset)
{
    printf("Tell\n");

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_TELL))
        return __WASI_ENOTCAPABLE;

    int64_t off = lseek(fd, 0, SEEK_CUR);

    if(off == -1)
        return WASI_ERRNO();

    *newoffset = off;

    return __WASI_ESUCCESS;
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
    if(!file_sandbox.has_fd(dirfd))
        return __WASI_EBADF;

    printf("Openasdfasdf\n");

    if(path_len == 0)
        return __WASI_EACCES;

    std::string pth = make_str(path, path_len);

    file_desc out;
    __wasi_errno_t err = file_sandbox.make_file(dirfd, pth, out, oflags);

    if(err != __WASI_ESUCCESS)
        return err;

    *fd = out.fd;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_path_unlink_file(__wasi_fd_t fd, const wasi_ptr_t<char> path, wasi_size_t path_len)
{
    printf("Start unlink\n");

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    printf("Unlink?\n");

    ///? TODO: Which permissions should this use?
    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_PATH_UNLINK_FILE))
        return __WASI_ENOTCAPABLE;

    printf("Is capable unlink\n");

    std::string their_path = make_str(path, path_len);

    const file_desc& v1 = file_sandbox.files[fd];

    std::filesystem::path p1 = std::filesystem::path(v1.relative_path) / std::filesystem::path(their_path);

    printf("Checking access\n");

    if(!file_sandbox.path_in_sandbox(p1))
        return __WASI_EACCES;

    if(!std::filesystem::is_regular_file(p1))
    {
        if(std::filesystem::is_directory(p1))
            return __WASI_EISDIR;

        return __WASI_EINVAL;
    }

    int res = unlink(p1.string().c_str());

    printf("Doing res %i\n", res);

    if(res != 0)
        return WASI_ERRNO();

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_path_remove_directory(__wasi_fd_t fd, const wasi_ptr_t<char> path, uint32_t path_len)
{
    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    ///? TODO: Which permissions should this use?
    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_PATH_REMOVE_DIRECTORY))
        return __WASI_ENOTCAPABLE;

    std::string their_path = make_str(path, path_len);

    const file_desc& v1 = file_sandbox.files[fd];

    std::filesystem::path p1 = std::filesystem::path(v1.relative_path) / std::filesystem::path(their_path);

    if(!file_sandbox.path_in_sandbox(p1))
        return __WASI_EACCES;

    if(!std::filesystem::is_directory(p1))
        return __WASI_EINVAL;

    int res = remove(p1.string().c_str());

    if(res != 0)
        return WASI_ERRNO();

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_path_rename(__wasi_fd_t old_fd,
                                    const wasi_ptr_t<char>old_path,
                                    wasi_size_t old_path_len,
                                    __wasi_fd_t new_fd,
                                    const wasi_ptr_t<char>new_path,
                                    wasi_size_t new_path_len)
{
    printf("PATH RENAME\n");

    if(!file_sandbox.has_fd(old_fd))
        return __WASI_EBADF;

    if(!file_sandbox.has_fd(new_fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(old_fd, __WASI_RIGHT_PATH_RENAME_SOURCE))
        return __WASI_ENOTCAPABLE;

    if(!file_sandbox.can_fd(new_fd, __WASI_RIGHT_PATH_RENAME_TARGET))
        return __WASI_ENOTCAPABLE;

    const file_desc& v1 = file_sandbox.files[old_fd];
    const file_desc& v2 = file_sandbox.files[new_fd];

    std::filesystem::path p1 = std::filesystem::path(v1.relative_path) / std::filesystem::path(make_str(old_path, old_path_len));
    std::filesystem::path p2 = std::filesystem::path(v2.relative_path) / std::filesystem::path(make_str(new_path, new_path_len));

    printf("here\n");

    if(!file_sandbox.path_in_sandbox(p1) || !file_sandbox.path_in_sandbox(p2))
        return __WASI_EACCES;

    int rval = rename(p1.string().c_str(), p2.string().c_str());

    printf("Try rename\n");

    //std::cout << "from " << p1.string() << " to " << p2.string() << std::endl;

    if(rval != 0)
        return WASI_ERRNO();

    //std::cout << "Renamed " << p1 << " to " << p2 << std::endl;

    return __WASI_ESUCCESS;
}

__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd, const wasi_ptr_t<__wasi_iovec_t> iovs, wasi_size_t iovs_len, wasi_ptr_t<uint32_t> nread)
{
    printf("Read %i\n", fd);

    *nread = 0;

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_READ))
        return __WASI_ENOTCAPABLE;

    for(size_t i=0; i < iovs_len; i++)
    {
        __wasi_iovec_t single = iovs[i];

        const wasi_ptr_t<void> buf = single.buf;

        wasi_ptr_t<char> tchr(0);
        tchr.val = buf.val;

        printf("BUF LEN %i\n", single.buf_len);

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

    *nwritten = 0;

    if(!file_sandbox.has_fd(fd))
        return __WASI_EBADF;

    if(!file_sandbox.can_fd(fd, __WASI_RIGHT_FD_WRITE))
        return __WASI_ENOTCAPABLE;

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
