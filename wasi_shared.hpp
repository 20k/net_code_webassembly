#ifndef WASI_SHARED_HPP_INCLUDED
#define WASI_SHARED_HPP_INCLUDED

std::variant

struct wasi_prestat_t
{
    enum type
    {
        WASI_PREOPENTYPE_DIR,
        COUNT,
    };

    type mtype = COUNT;
    size_t u_pr_name_len = 0;
};

using wasi_fd_t = uint32_t;

enum class wasi_advice_t : uint8_t
{
    WASI_ADVICE_DONTNEED,
    WASI_ADVICE_NOREUSE,
    WASI_ADVICE_NORMAL,
    WASI_ADVICE_RANDOM,
    WASI_ADVICE_SEQUENTIAL,
    WASI_ADVICE_WILLNEED,
};

struct wasi_ciovec_t
{
    const void* buf = nullptr;
    size_t buf_len = 0;
};

enum class wasi_clockid_t : uint32_t
{
    WASI_CLOCK_MONOTONIC,
    WASI_CLOCK_PROCESS_CPUTIME_ID,
    WASI_CLOCK_REALTIME,
    WASI_CLOCK_THREAD_CPUTIME_ID
};

using wasi_device_t = uint64_t;

#define WASI_DIRCOOKIE_START -1
using wasi_dircookie_t = uint64_t;

using wasi_inode_t = uint64_t;

enum class wasi_filetype_t : uint8_t
{
    WASI_FILETYPE_UNKNOWN,
    WASI_FILETYPE_BLOCK_DEVICE,
    WASI_FILETYPE_CHARACTER_DEVICE,
    WASI_FILETYPE_DIRECTORY,
    WASI_FILETYPE_REGULAR_FILE,
    WASI_FILETYPE_SOCKET_DGRAM,
    WASI_FILETYPE_SOCKET_STREAM,
    WASI_FILETYPE_SYMBOLIC_LINK,
};

struct wasi_dirent_t
{
    wasi_dircookie_t d_next = WASI_DIRCOOKIE_START;
    wasi_inode_t d_ino = -1;
    uint32_t d_namlen = 0;
    wasi_filetype_t d_type = wasi_filetype_t::WASI_FILETYPE_UNKNOWN;
};

enum class wasi_errno_t : uint16_t
{
    WASI_ESUCCESS,
    WASI_E2BIG,
    WASI_EACCES,
    WASI_EADDRINUSE,
    WASI_EADDRNOTAVAIL,
    WASI_EAFNOSUPPORT,
    WASI_EAGAIN,
    WASI_EALREADY,
    WASI_EBADF,
    WASI_EBADMSG,
    WASI_EBUSY,
    WASI_ECANCELED,
    WASI_ECHILD,
    WASI_ECONNABORTED,
    WASI_ECONNREFUSED,
    WASI_ECONNRESET,
    WASI_EDEADLK,
    WASI_EDESTADDRREQ,
    WASI_EDOM,
    WASI_EDQUOT,
    WASI_EEXIST,
    WASI_EFAULT,
    WASI_EFBIG,
    WASI_EHOSTUNREACH,
    WASI_EIDRM,
    WASI_EILSEQ,
    WASI_EINPROGRESS,
    WASI_EINTR,
    WASI_EINVAL,
    WASI_EIO,
    WASI_EISCONN,
    WASI_EISDIR,
    WASI_ELOOP,
    WASI_EMFILE,
    WASI_EMLINK,
    WASI_EMSGSIZE,
    WASI_EMULTIHOP,
    WASI_ENAMETOOLONG,
    WASI_ENETDOWN,
    WASI_ENETRESET,
    WASI_ENETUNREACH,
    WASI_ENFILE,
    WASI_ENOBUFS,
    WASI_ENODEV,
    WASI_ENOENT,
    WASI_ENOEXEC,
    WASI_ENOLCK,
    WASI_ENOLINK,
    WASI_ENOMEM,
    WASI_ENOMSG,
    WASI_ENOPROTOOPT,
    WASI_ENOSPC,
    WASI_ENOSYS,
    WASI_ENOTCONN,
    WASI_ENOTDIR,
    WASI_ENOTEMPTY,
    WASI_ENOTRECOVERABLE,
    WASI_ENOTSOCK,
    WASI_ENOTSUP,
    WASI_ENOTTY,
    WASI_ENXIO,
    WASI_EOVERFLOW,
    WASI_EOWNERDEAD,
    WASI_EPERM,
    WASI_EPIPE,
    WASI_EPROTO,
    WASI_EPROTONOSUPPORT,
    WASI_EPROTOTYPE,
    WASI_ERANGE,
    WASI_EROFS,
    WASI_ESPIPE,
    WASI_ESRCH,
    WASI_ESTALE,
    WASI_ETIMEDOUT,
    WASI_ETXTBSY,
    WASI_EXDEV,
    WASI_ENOTCAPABLE,
};

using wasi_userdata_t = uint64_t;

enum class wasi_eventtype_t : uint8_t
{
    WASI_EVENTTYPE_CLOCK,
    WASI_EVENTTYPE_FD_READ,
    WASI_EVENTTYPE_FD_WRITE,
    COUNT
};

using wasi_filesize_t = uint64_t;

enum class wasi_eventrwflags_t : uint16_t
{
    WASI_EVENT_FD_READWRITE_HANGUP,
    COUNT
};

template<typename... T>
struct type_check
{
    int which = 0;

    constexpr int max_num = sizeof...(T);
};

struct wasi_event_t
{
    wasi_userdata_t userdata = -1;
    wasi_errno_t error = wasi_errno_t::WASI_ESUCCESS;
    wasi_eventtype_t type = wasi_eventtype_t::COUNT;
};

void wasi_fd_prestat_dir_name();

#endif // WASI_SHARED_HPP_INCLUDED
