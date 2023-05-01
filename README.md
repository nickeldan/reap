*"We will encourage you to develop the three great virtues of a programmer: laziness, impatience, and hubris.”* - Larry Wall

The REAP library provides Ridiculously Easy Access to Procfs.

API
===

You can access all of REAP's functionality by

```c
#include <reap/reap.h>
```

Process info
------------

You can get basic information about a running process through the `reapGetProcInfo` function.  Its signature is

```c
int reapGetProcInfo(pid_t pid, reapProcInfo *info, char *exe_path, size_t path_size);
```

where `reapProcInfo` is defined as

```c
typedef struct reapProcInfo {
    pid_t pid;  // Process ID.
    pid_t ppid; // Parent process ID.
    pid_t tid;  // Task ID.
    uid_t uid;  // User ID.
    uid_t euid; // Effective user ID.
    gid_t gid;  // Group ID.
    gid_t egid; // Effective group ID.
} reapProcInfo;
```

`reapGetProcInfo` returns `REAP_RET_OK` if successful and an error code otherwise (defined in [reap/definitions.h](include/reap/definitions.h)).  In addition to the error codes defined in that file, various REAP functions can return negative values.  Such values will be equal to `-1 * errno`.

You can also pass a task ID (i.e., a value returned by the `gettid` system call) as `pid`.

If `exe_path` is not `NULL` and is of size `path_size`, then it will be populated with the path to the process' executable.

Error messages
--------------

When an error occurs, you can get a description of the error by

```c
char *reapGetError(void);
```

The referenced buffer (which is unique to each thread) will be of size `REAP_ERROR_BUFFER_SIZE` (defined in [reap/config.h](include/reap/config.h)).

Process iteration
-----------------

A `reapProcIterator` can be used to iterate over all of the running processes (at least, those for which the user has permission to access).

First, the iterator has to be created:

```c
int ret;
reapProcIterator *iterator;

ret = reapProcIteratorCreate(&iterator);
if ( ret != REAP_RET_OK ) {
    // handle the error
}
```

After that, we can repeatedly pull results from the iterator with

```c
int reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info, char *exe_path, size_t path_size);
```

where the last three arguments are the same as for `reapGetProcInfo`.

This function returns `REAP_RET_OK` when yielding a result, `REAP_RET_DONE` when the iterator has been exhausted, and an error code otherwise.

The iterator must be destroyed when it is no longer needed:

```c
reapProcIteratorDestroy(iterator);
```

File descriptor iteration
-------------------------

A `reapFdIterator` can be used to iterate over a process' open file descriptors.

First, the iterator has to be created:

```c
int ret;
reapFdIterator *iterator;

ret = reapFdIteratorCreate(some_pid, &iterator);
if ( ret != REAP_RET_OK ) {
    // handle the error
}
```

After that, we can repeatedly acquire file descriptor information with

```c
int reapFdIteratorNext(const reapFdIterator *iterator, reapFdResult *result, char *file, size_t file_size);
```

where `reapFdResult` is defined as

```c
typedef struct reapFdResult {
    int fd;
    dev_t device;
    ino_t inode;
    mode_t mode;
} reapFdResult;
```

This function returns `REAP_RET_OK` when yielding a result, `REAP_RET_DONE` when the iterator has been exhausted, and an error code otherwise.

if `file` is not `NULL` and is of size `file_size`, then it will be populated with the name of the underlying file. 

The iterator must be destroyed when it is no longer needed:

```c
reapFdIteratorDestroy(iterator);
```

Memory map iteration
--------------------

A `reapMapIterator` can be used to iterate over a process' mapped memory segments.

First, the iterator has to be created:

```c
int ret;
reapMapIterator *iterator;

ret = reapMapIteratorCreate(some_pid, &iterator);
if ( ret != REAP_RET_OK ) {
    // handle the error
}
```

After that, we can repeatedly acquire mapped memory information with

```c
int reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result, char *name, size_t name_size);
```

where `reapMapResult` is defined as

```c
typedef struct reapMapResult {
    unsigned long long start; // The start address of the memory segment.
    unsigned long long end; // The end address of the memory segment.
    unsigned long long offset; // The offset of the memory segment within the referent file (if any).
    int permissions; // The permissions of the memory segment.
    dev_t device; // The device number of the referent file (if any).
    ino_t inode; // The inode number of the referent file (if any).
} reapMapResult;
```

The permissions are some bitwise-OR combination of `PROT_READ`, `PROT_WRITE`, and `PROT_EXEC` from sys/mman.h.

This function returns `REAP_RET_OK` when yielding a result, `REAP_RET_DONE` when the iterator has been exhausted, and an error code otherwise.

If `name` is not `NULL` and is of size `name_size`, then it will be populated with the name of the memory segment.

The iterator must be destroyed when it is no longer needed:

```c
reapMapIteratorDestroy(iterator);
```

Thread iteration
----------------

A `reapThreadIterator` can be used to iterate over a process' threads.

First, the iterator has to be created:

```c
int ret;
reapThreadIterator *iterator;

ret = reapThreadIteratorCreate(some_pid, &iterator);
if ( ret != REAP_RET_OK ) {
    // handle the error
}
```

After initialization, we can repeatedly acquire thread IDs with

```c
int reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread);
```

This function returns `REAP_RET_OK` when yielding a result, `REAP_RET_DONE` when the iterator has been exhausted, and an error code otherwise.

The iterator must be destroyed when it is no longer needed:

```c
reapThreadIteratorDestroy(iterator);
```

Socket iteration
----------------

A `reapNetIterator` can be used to iterate over the open sockets in the network namespace.

First, the iterator has to be created:

```c
int ret;
reapNetIterator *iterator;

ret = reapNetIteratorCreate(0, &iterator);
if ( ret != REAP_RET_OK ) {
    // handle the error
}
```

The first parameter to the function holds zero or more flags combined with bitwise-OR.  The available flags are

* `REAP_NET_FLAG_UDP`: Find UDP sockets.
* `REAP_NET_FLAG_IPV6`: Find IPv6 sockets.
* `REAP_NET_FLAG_DOMAIN`: Find Unix domain sockets.  If this flag is specified, then all other flags are ignored.

By default, the iterator will find TCP sockets over IPv4.

After initialization, we can repeatedly acquire socket information with

```c
int reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result);
```

where `reapNetResult` is defined as

```c
typedef struct reapNetResult {
    union {
        struct { // For IP sockets.
            reapNetPeer local;
            reapNetPeer remote;
        };
        struct { // For Unix domain sockets.
            char path[108]; // Actually, the size is the same as that of the sun_path field of struct sockaddr_un.
            int socket_type; // E.g., SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET.
            unsigned int connected : 1;
        };
    };
    ino_t inode; // The inode number of the socket.
    unsigned int flags; // The flags that were passed to reapNetIteratorCreate.
} reapNetResult;
```

where `reapNetPeer` is defined as

```c
typedef struct reapNetPeer {
    uint16_t port;
    uint8_t address[16];
} reapNetPeer;
```

If representing a Unix domain socket which is connected to an abstract socket address (i.e., where the first character of the path is a null byte), then all of the null bytes in the path will be replaced by `'@'`s.

This function returns `REAP_RET_OK` when yielding a result, `REAP_RET_DONE` when the iterator has been exhausted, and an error code otherwise.

The iterator must be destroyed when it is no longer needed:

```c
reapNetIteratorDestroy(iterator);
```

Building REAP
=============

Shared and static libraries are built using make.  Adding `debug=yes` to the make invocation will disable optimization and build the libraries with debugging symbols.

You can also include REAP in a larger project by including make.mk.  Before doing so, however, the `REAP_DIR` variable must be set to the location of the REAP directory.  You can also tell make where to place the shared and static libraries by defining the `REAP_LIB_DIR` variable (defaults to `$(REAP_DIR)`). Similarly, you can define the `REAP_OBJ_DIR` variable which tells make where to place the object files (defaults to `$(REAP_DIR)/src`).

make.mk adds a target to the `CLEAN_TARGETS` variable.  This is so that implementing

```make
clean: $(CLEAN_TARGETS)
    ...
```

in your project's Makefile will cause REAP to be cleaned up as well.

The `CLEAN_TARGETS` variable should be added to `.PHONY` if you're using GNU make.

make.mk defines the variables `REAP_SHARED_LIBRARY` and `REAP_STATIC_LIBRARY` which contain the paths of the specified libraries.

Feature flags
-------------

By defining various preprocessor variables, you can disable some of REAP's features at compile time.  The available options are

* `REAP_NO_PROC` (sets `REAP_NO_ITERATE_PROC` as well)
* `REAP_NO_ITERATE_PROC`
* `REAP_NO_ITERATE_FD`
* `REAP_NO_ITERATE_MAP`
* `REAP_NO_ITERATE_NET`
* `REAP_NO_ITERATE_THREAD`

Testing
=======

Testing can be performed through the [Scrutiny framework](https://github.com/nickeldan/scrutiny).  After installing at least version 0.5.0 of the framework, you can run REAP's tests by

```sh
make tests
```
