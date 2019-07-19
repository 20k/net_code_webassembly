#ifndef WASI_SHARED_HPP_INCLUDED
#define WASI_SHARED_HPP_INCLUDED

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

struct wasi_event_t
{
    wasi_userdata_t userdata = -1;
    wasi_errno_t error = wasi_errno_t::WASI_ESUCCESS;
    wasi_eventtype_t type = wasi_eventtype_t::COUNT;

    union fd_readwrite
    {
        wasi_filesize_t nbytes;
        wasi_eventrwflags_t flags;
    } u;
};

using wasi_exitcode_t = uint32_t;

enum class wasi_fdflags_t : uint16_t
{
    WASI_FDFLAG_APPEND,
    WASI_FDFLAG_DSYNC,
    WASI_FDFLAG_NONBLOCK,
    WASI_FDFLAG_RSYNC,
    WASI_FDFLAG_SYNC,
};

enum class wasi_rights_t : uint64_t
{
    WASI_RIGHT_FD_DATASYNC,
    WASI_RIGHT_FD_READ,
    WASI_RIGHT_FD_SEEK,
    WASI_RIGHT_FD_FDSTAT_SET_FLAGS,
    WASI_RIGHT_FD_SYNC,
    WASI_RIGHT_FD_TELL,
    WASI_RIGHT_FD_WRITE,
    WASI_RIGHT_FD_ADVISE,
    WASI_RIGHT_FD_ALLOCATE,
    WASI_RIGHT_PATH_CREATE_DIRECTORY,
    WASI_RIGHT_PATH_CREATE_FILE,
    WASI_RIGHT_PATH_LINK_SOURCE,
    WASI_RIGHT_PATH_LINK_TARGET,
    WASI_RIGHT_PATH_OPEN,
    WASI_RIGHT_FD_READDIR,
    WASI_RIGHT_PATH_READLINK,
    WASI_RIGHT_PATH_RENAME_SOURCE,
    WASI_RIGHT_PATH_RENAME_TARGET,
    WASI_RIGHT_PATH_FILESTAT_GET,
    WASI_RIGHT_PATH_FILESTAT_SET_SIZE,
    WASI_RIGHT_PATH_FILESTAT_SET_TIMES,
    WASI_RIGHT_FD_FILESTAT_GET,
    WASI_RIGHT_FD_FILESTAT_SET_SIZE,
    WASI_RIGHT_FD_FILESTAT_SET_TIMES,
    WASI_RIGHT_PATH_SYMLINK,
    WASI_RIGHT_PATH_UNLINK_FILE,
    WASI_RIGHT_PATH_REMOVE_DIRECTORY,
    WASI_RIGHT_POLL_FD_READWRITE,
    WASI_RIGHT_SOCK_SHUTDOWN,
    NONE,
};

struct wasi_fdstat_t
{
    wasi_filetype_t fs_filetype = wasi_filetype_t::WASI_FILETYPE_UNKNOWN;
    wasi_fdflags_t fs_flags = wasi_fdflags_t::WASI_FDFLAG_SYNC;
    wasi_rights_t fs_rights_base = wasi_rights_t::NONE;
    wasi_rights_t fs_rights_inheriting = wasi_rights_t::NONE;
};

using wasi_filedelta_t = int64_t;
using wasi_device_t = uint64_t;
using wasi_linkcount_t = uint32_t;
using wasi_timestamp_t = uint64_t;

struct wasi_filestat_t
{
    wasi_device_t st_dev = -1;
    wasi_inode_t st_ino = -1;
    wasi_filetype_t st_filetype = wasi_filetype_t::WASI_FILETYPE_UNKNOWN;
    wasi_linkcount_t st_nlink = 0;
    wasi_filesize_t st_size = 0;
    wasi_timestamp_t st_atim = 0;
    wasi_timestamp_t st_mtim = 0;
    wasi_timestamp_t st_ctim = 0;
};

enum class wasi_fstflags_t : uint16_t
{
    WASI_FILESTAT_SET_ATIM,
    WASI_FILESTAT_SET_ATIM_NOW,
    WASI_FILESTAT_SET_MTIM,
    WASI_FILESTAT_SET_MTIM_NOW,
};

struct wasi_iovec_t
{
    void* buf = nullptr;
    size_t buf_len = 0;
};

enum class wasi_lookupflags_t : uint32_t
{
    WASI_LOOKUP_SYMLINK_FOLLOW
};

enum class wasi_oflags_t : uint16_t
{
    WASI_O_CREAT,
    WASI_O_DIRECTORY,
    WASI_O_EXCL,
    WASI_O_TRUNC,
};

enum class wasi_preopentype_t : uint8_t
{
    WASI_PREOPENTYPE_DIR,
    NONE
};

struct wasi_prestat_t
{
    wasi_preopentype_t mtype = wasi_preopentype_t::NONE;

    union u_t
    {
        size_t pr_name_len;
    } u;
};

enum class wasi_riflags_t : uint16_t
{
    WASI_SOCK_RECV_PEEK,
    WASI_SOCK_RECV_WAITALL,
};

enum class wasi_roflags_t : uint16_t
{
    WASI_SOCK_RECV_DATA_TRUNCATED,
};

enum class wasi_sdflags_t : uint8_t
{
    WASI_SHUT_RD,
    WASI_SHUT_WR,
};

enum class wasi_siflags_t : uint16_t
{

};

void wasi_fd_prestat_dir_name();

#endif // WASI_SHARED_HPP_INCLUDED
