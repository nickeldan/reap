#include <scrutiny/scrutiny.h>

void *
procSetup(void *global_ctx);

void
procCleanup(void *group_ctx);

SCR_TEST_FN(find_self);
SCR_TEST_FN(get_self_path);
SCR_TEST_FN(iterate_procs);

SCR_TEST_FN(iterate_threads);

int
main()
{
    int ret;
    scrRunner *runner;
    scrGroup *group;

    runner = scrRunnerCreate();

    group = scrGroupCreate(runner, procSetup, procCleanup);
    scrGroupAddTest(group, "Get proc info", find_self, 0, 0);
    scrGroupAddTest(group, "Get self path", get_self_path, 0, 0);
    scrGroupAddTest(group, "Proc iterator", iterate_procs, 5, 0);

    group = scrGroupCreate(runner, NULL, NULL);
    scrGroupAddTest(group, "Thread iterator", iterate_threads, 5, 0);

    ret = scrRunnerRun(runner, 0, NULL, NULL);
    scrRunnerDestroy(runner);
    return ret;
}
