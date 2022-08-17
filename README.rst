====
REAP
====

:Author: Daniel Walker
:Version: 0.8.4
:Date: 2022-08-16

*"We will encourage you to develop the three great virtues of a programmer: laziness, impatience, and hubris.”* - Larry Wall

The REAP library provides Ridiculously Easy Access to Procfs.

API
===

You can access all of REAP's functionality by

.. code-block:: c

    #include <reap/reap.h>

Process info
------------

You can get basic information about a running process through the **reapGetProcInfo** function.  Its
signature is

.. code-block:: c

    int reapGetProcInfo(pid_t pid, reapProcInfo *info);

where **reapProcInfo** is defined as

.. code-block:: c

    typedef struct reapProcInfo {
        pid_t pid;  // Process ID.
        pid_t ppid; // Parent process ID.
        pid_t tgid; // Task group ID.
        uid_t uid;  // User ID.
        uid_t euid; // Effective user ID.
        gid_t gid;  // Group ID.
        gid_t egid; // Effective group ID.
        char exe[REAP_SHORT_PATH_SIZE]; // Path of the file being run.
    } reapProcInfo;

where **REAP_SHORT_PATH_SIZE** is a preprocessor variable defined in reap/config.h.

**reapGetProcInfo** returns **REAP_RET_OK** if successful and an error code otherwise (defined in
reap/definitions.h).

Error strings
-------------

An error code, such as one returned from **reapProcGetInfo**, can be converted into a string by

.. code-block:: c

    const char *reapErrorString(int value);

In addition, if the **REAP_USE_ERROR_BUFFER** preprocessor variable is defined (see reap/config.h), a buffer
will be kept in thread-local storage which will be populated with a more descriptive message whenever an
error occurs.  This buffer can be accessed with

.. code-block:: c

    char *reapGetError(void);

Process iteration
-----------------

A **reapProcIterator** can be used to iterate over all of the running processes (at least, those for which
the user has permission to access).

First, the iterator has to be initialized.

.. code-block:: c

    int ret;
    reapProcIterator iterator;

    ret = reapProcIteratorInit(&iterator);
    if ( ret != REAP_RET_OK ) {
        // handle the error
    }

After that, we can repeatedly pull results from the iterator with

.. code-block:: c

    int reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info);

This function returns **REAP_RET_OK** when yielding a result, **REAP_RET_DONE** when the iterator has been
exhausted, and an error code otherwise.

The iterator must be closed when it is no longer needed.

.. code-block:: c

    void reapProcIteratorClose(reapProcIterator *iterator);

File descriptor iteration
-------------------------

A **reapFdIterator** can be used to iterate over a process' open file descriptors.

First, the iterator has to be initialized.

.. code-block:: c

    int ret;
    reapFdIterator iterator;

    ret = reapFdIteratorInit(some_pid, &iterator);
    if ( ret != REAP_RET_OK ) {
        // handle the error
    }

After that, we can repeatedly acquire file descriptor information with

.. code-block:: c

    int reapFdIteratorNext(const reapFdIterator *iterator, reapFdResult *result);

where **reapFdResult** is defined as

.. code-block:: c

    typedef struct reapFdResult {
        int fd;
        dev_t device;
        ino_t inode;
        mode_t mode;
        char file[REAP_SHORT_PATH_SIZE];
    } reapFdResult;

**reapFdIteratorNext** returns **REAP_RET_OK** when yielding a result, **REAP_RET_DONE** when the iterator
has been exhausted, and an error code otherwise.

The iterator must be closed when it is no longer needed.

.. code-block:: c

    void reapFdIteratorClose(reapFdIterator *iterator);

Memory map iteration
--------------------

A **reapMapIterator** can be used to iterate over a process' mapped memory sections.

First, the iterator has to be initialized.

.. code-block:: c

    int ret;
    reapMapIterator iterator;

    ret = reapMapIteratorInit(some_pid, &iterator);
    if ( ret != REAP_RET_OK ) {
        // handle the error
    }

After that, we can repeatedly acquire mapped memory information with

.. code-block:: c

    int reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result);

where **reapMapResult** is defined as

.. code-block:: c

    typedef struct reapMapResult {
        unsigned long start; // The start address of the memory section.
        unsigned long end; // The end address of the memory section.
        unsigned int offset; // The offset of the memory section within the referent file (if any).
        int permissions; // The permissions of the memory section.
        dev_t device; // The device number of the referent file (if any).
        ino_t inode; // The inode of the referent file (if any).
        char name[REAP_SHORT_PATH_SIZE]; // The name of the memory section (if any).
    } reapMapResult;

The permissions are some bitwise-OR combination of **PROT_READ**, **PROT_WRITE**, and **PROT_EXEC** from
sys/mman.h.

**reapMapIteratorNext** returns **REAP_RET_OK** when yielding a result, **REAP_RET_DONE** when the iterator
has been exhausted, and an error code otherwise.

The iterator must be closed when it is no longer needed.

.. code-block:: c

    void reapMapIteratorClose(reapMapIterator *iterator);

Thread iteration
----------------

A **reapThreadIterator** can be used to iterate over a process' threads.

First, the iterator has to be intialized.

.. code-block:: c

    int ret;
    reapThreadIterator iterator;

    ret = reapThreadIteratorInit(some_pid, &iterator);
    if ( ret != REAP_RET_OK ) {
        // handle the error
    }

After initialization, we can repeatedly acquire thread PIDs with

.. code-block:: c

    int reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread);

**reapThreadIteratorNext** returns **REAP_RET_OK** when yielding a result, **REAP_RET_DONE** when the
iterator has been exhausted, and an error code otherwise.

The iterator must be closed when it is no longer needed.

.. code-block:: c

    void reapThreadIteratorClose(reapThreadIterator *iterator);

Socket iteration
----------------

A **reapNetIterator** can be used to iterate over the open sockets in the network namespace.

First, the iterator has to be initalized.

.. code-block:: c

    int ret;
    reapNetIterator iterator;

    ret = reapNetIteratorInit(&iterator, 0);
    if ( ret != REAP_RET_OK ) {
        // handle the error
    }

The second parameter to the initializer holds zero or more flags combined with bitwise-OR.  The available
flags are

* **REAP_NET_FLAG_UDP**: Find UDP sockets.
* **REAP_NET_FLAG_IPV6**: Find IPv6 sockets.
* **REAP_NET_FLAG_DOMAIN**: Find Unix domain sockets.  If this flag is specified, then all other flags are ignored.

By default, the iterator will find TCP sockets over IPv4.

After initialization, we can repeatedly acquire socket information with

.. code-block:: c

    int reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result);

where **reapNetResult** is defined as

.. code-block:: c

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
        ino_t inode;
        unsigned int flags; // The flags that were passed to reapNetIteratorInit.
    } reapNetResult;

where **reapNetPeer** is defined as

.. code-block:: c

    typedef struct reapNetPeer {
        uint16_t port;
        uint8_t address[16];
    } reapNetPeer;

If representing a Unix domain socket which is connected to an abstract socket address (i.e., where the first
character of the path is a null byte), then the first character of **path** will be **'@'**.

**reapNetIteratorNext** returns **REAP_RET_OK** when yielding a result, **REAP_RET_DONE** when the iterator
has been exhausted, and an error code otherwise.

The iterator must be closed when it is no longer needed.

.. code-block:: c

    void reapNetIteratorClose(reapNetIterator *iterator);

Building REAP
=============

Shared and static libraries are built using make.  Adding "debug=yes" to the make invocation will disable
optimization and build the libraries with debugging symbols.

You can also include REAP in a larger project by including make.mk.  Before doing so, however, the
**REAP_DIR** variable must be set to the location of the REAP directory.  You can also tell make where to
place the shared and static libraries by defining the **REAP_LIB_DIR** variable (defaults to **REAP_DIR**).
Similarly, you can define the **REAP_OBJ_DIR** variable which tells make where to place the object files
(defaults to **REAP_DIR**/src).

make.mk adds a target to the **CLEAN_TARGETS** variable.  This is so that implementing

.. code-block:: make

    clean: $(CLEAN_TARGETS)
        ...

in your project's Makefile will cause REAP to be cleaned up as well.

The **CLEAN_TARGETS** variable should be added to **.PHONY** if you're using GNU make.

make.mk defines the variables **REAP_SHARED_LIBRARY** and **REAP_STATIC_LIBRARY** which contain the paths of
the specified libraries.

Testing
=======

To build executables which test basic functionality of the library, run

.. code-block:: sh

    make tests
