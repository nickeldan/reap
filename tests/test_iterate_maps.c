#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

void
iterateMaps(void)
{
    int ret;
    struct stat fs;
    reapMapIterator *iterator;
    reapMapResult result;
    char name[PATH_MAX];

    if (stat("/proc/self/exe", &fs) != 0) {
        SCR_FAIL("stat (/proc/self/exe): %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapMapIteratorCreate(getpid(), &iterator), REAP_RET_OK);

    while ((ret = reapMapIteratorNext(iterator, &result, name, sizeof(name))) == REAP_RET_OK) {
        bool r, w, x;

        r = result.permissions & PROT_READ;
        w = result.permissions & PROT_WRITE;
        x = result.permissions & PROT_EXEC;

        SCR_LOG("%llx-%llx %c%c%c %08llx %02x:%02x %lu\t%s", result.start, result.end, r ? 'r' : '-',
                w ? 'w' : '-', x ? 'x' : '-', result.offset, major(result.device), minor(result.device),
                (unsigned long)result.inode, name);

        if (result.inode == fs.st_ino && result.device == fs.st_dev) {
            reapMapIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_FAIL("reapMapIteratorNext: %s", reapGetError());
    }

    SCR_FAIL("Couldn't find self mapped into virtual memory");
}
