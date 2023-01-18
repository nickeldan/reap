#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

struct threadArgs {
    pthread_mutex_t lock;
    pid_t tid;
    sem_t sem;
};

static void *
threadFunc(void *args)
{
    struct threadArgs *args_struct = args;

    args_struct->tid = syscall(__NR_gettid);
    sem_post(&args_struct->sem);

    pthread_mutex_lock(&args_struct->lock);
    pthread_mutex_unlock(&args_struct->lock);

    return NULL;
}

void
iterate_threads(void)
{
    int ret;
    pthread_t thread;
    pid_t pid, tid, tval;
    bool found_self = false, found_thread = false;
    struct threadArgs args = {
        .lock = PTHREAD_MUTEX_INITIALIZER,
    };
    reapThreadIterator *iterator;

    if (sem_init(&args.sem, 0, 0) != 0) {
        SCR_ERROR("sem_init: %s", strerror(errno));
    }
    pthread_mutex_lock(&args.lock);

    ret = pthread_create(&thread, NULL, threadFunc, &args);
    if (ret != 0) {
        SCR_ERROR("pthread_create: %s", strerror(ret));
    }
    sem_wait(&args.sem);
    sem_destroy(&args.sem);

    pid = getpid();
    tid = syscall(__NR_gettid);
    SCR_LOG("Main thread = %li", (long)tid);
    SCR_LOG("Thread = %li", (long)args.tid);

    if (reapThreadIteratorCreate(pid, &iterator) != REAP_RET_OK) {
        SCR_ERROR("reapThreadIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapThreadIteratorNext(iterator, &tval)) == REAP_RET_OK) {
        SCR_LOG("Found thread: %li", (long)tval);
        if (tval == tid) {
            found_self = true;
        }
        else if (tval == args.tid) {
            reapProcInfo info;

            found_thread = true;

            if (reapGetProcInfo(tval, &info, NULL, 0) != REAP_RET_OK) {
                SCR_ERROR("reapGetProcInfo: %s", reapGetError());
            }
            SCR_ASSERT_EQ(info.pid, pid);
            SCR_ASSERT_EQ(info.tid, tval);
        }
    }

    reapThreadIteratorDestroy(iterator);
    pthread_mutex_unlock(&args.lock);
    pthread_join(thread, NULL);

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapThreadIteratorNext: %s", reapGetError());
    }

    if (!found_self) {
        SCR_ERROR("Did not find own TID");
    }

    if (!found_thread) {
        SCR_ERROR("Did not find TID of side thread");
    }
}
