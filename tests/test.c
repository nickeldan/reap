#include <scrutiny/scrutiny.h>

void *
procSetup(void *global_ctx);
void
procCleanup(void *group_ctx);
void
find_self(void);
void
get_self_path(void);
void
iterate_procs(void);

void *
fdSetup(void *global_ctx);
void
fdCleanup(void *group_ctx);
void
iterate_fds(void);

void *
tcpIpv4Setup(void *global_ctx);
void
socketCleanup(void *group_ctx);
void
iterate_tcp_ipv4_find_server(void);
void
iterate_tcp_ipv4_find_client(void);

void *
tcpIpv6Setup(void *global_ctx);
void
iterate_tcp_ipv6_find_server(void);
void
iterate_tcp_ipv6_find_client(void);

void
iterate_threads(void);
void
iterate_maps(void);

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

    group = scrGroupCreate(runner, fdSetup, fdCleanup);
    scrGroupAddTest(group, "File descriptor iterator", iterate_fds, 5, 0);

    group = scrGroupCreate(runner, tcpIpv4Setup, socketCleanup);
    scrGroupAddTest(group, "Net iterator find TCP IPv4 server", iterate_tcp_ipv4_find_server, 5, 0);
    scrGroupAddTest(group, "Net iterator find TCP IPv4 client", iterate_tcp_ipv4_find_client, 5, 0);

    group = scrGroupCreate(runner, tcpIpv6Setup, socketCleanup);
    scrGroupAddTest(group, "Net iterator find TCP IPv6 server", iterate_tcp_ipv6_find_server, 5, 0);
    scrGroupAddTest(group, "Net iterator find TCP IPv6 client", iterate_tcp_ipv6_find_client, 5, 0);

    group = scrGroupCreate(runner, NULL, NULL);
    scrGroupAddTest(group, "Thread iterator", iterate_threads, 5, 0);
    scrGroupAddTest(group, "Map iterator", iterate_maps, 5, 0);

    ret = scrRunnerRun(runner, NULL, NULL);
    scrRunnerDestroy(runner);
    return ret;
}
