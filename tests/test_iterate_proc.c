#include <errno.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

void *
procSetup(void *global_ctx)
{
    char *path;

    (void)global_ctx;

    SCR_ASSERT_PTR_NEQ((path = malloc(PATH_MAX)), NULL);

    if (readlink("/proc/self/exe", path, PATH_MAX) < 0) {
        SCR_FAIL("readlink (/proc/self/exe): %s", strerror(errno));
    }

    return path;
}

void
procCleanup(void *group_ctx)
{
    free(group_ctx);
}

void
findSelf(void)
{
    pid_t pid;
    reapProcInfo info;

    pid = getpid();
    if (reapGetProcInfo(pid, &info, NULL, 0) != REAP_RET_OK) {
        SCR_FAIL("reapGetProcInfo: %s", reapGetError());
    }

    SCR_ASSERT_EQ(info.pid, pid);
    SCR_ASSERT_EQ(info.ppid, getppid());
    SCR_ASSERT_EQ(info.tid, pid);
    SCR_ASSERT_EQ(info.uid, getuid());
    SCR_ASSERT_EQ(info.euid, geteuid());
    SCR_ASSERT_EQ(info.gid, getgid());
    SCR_ASSERT_EQ(info.egid, getegid());
}

void
getSelfPath(void)
{
    pid_t pid;
    reapProcInfo info;
    char path[PATH_MAX];

    pid = getpid();
    if (reapGetProcInfo(pid, &info, path, sizeof(path)) != REAP_RET_OK) {
        SCR_FAIL("reapGetProcInfo: %s", reapGetError());
    }

    SCR_ASSERT_STR_EQ(path, SCR_GROUP_CTX());
}

void
iterateProcs(void)
{
    int ret;
    pid_t pid;
    reapProcIterator *iterator;
    reapProcInfo info;
    char path[PATH_MAX];

    pid = getpid();

    if (reapProcIteratorCreate(&iterator) != REAP_RET_OK) {
        SCR_FAIL("reapProcIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapProcIteratorNext(iterator, &info, path, sizeof(path))) == REAP_RET_OK) {
        SCR_LOG("%li: %s", (long)info.pid, path);
        if (info.pid == pid) {
            SCR_ASSERT_STR_EQ(path, SCR_GROUP_CTX());

            reapProcIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_FAIL("reapProcIteratorNext: %s", reapGetError());
    }

    SCR_FAIL("Could not find process %li", (long)pid);
}
