#include <scrutiny/scrutiny.h>

void *
procSetup(void *global_ctx);
void
procCleanup(void *group_ctx);
void
findSelf(void);
void
getSelfPath(void);
void
iterateProcs(void);

void *
fdSetup(void *global_ctx);
void
fdCleanup(void *group_ctx);
void
iterateFds(void);

void
serverCleanup(void *group_ctx);

void *
ipv4TcpServerSetup(void *global_ctx);
void
ipv4TcpFindServer(void);
void
ipv4TcpFindClient(void);

void *
ipv6TcpServerSetup(void *global_ctx);
void
ipv6TcpFindServer(void);
void
ipv6TcpFindClient(void);

void *
ipv4UdpServerSetup(void *global_ctx);
void
ipv4UdpFindServer(void);

void *
ipv6UdpServerSetup(void *global_ctx);
void
ipv6UdpFindServer(void);

void *
domainServerSetup(void *global_ctx);
void
domainServerCleanup(void *group_ctx);
void
domainFindServer(void);

void
iterateThreads(void);
void
iterateMaps(void);

int
main()
{
    scrGroup *group;

    group = scrGroupCreate(procSetup, procCleanup);
    scrGroupAddTest(group, "Get proc info", findSelf, 0, 0);
    scrGroupAddTest(group, "Get self path", getSelfPath, 0, 0);
    scrGroupAddTest(group, "Proc iterator", iterateProcs, 5, 0);

    group = scrGroupCreate(fdSetup, fdCleanup);
    scrGroupAddTest(group, "File descriptor iterator", iterateFds, 5, 0);

    group = scrGroupCreate(ipv4TcpServerSetup, serverCleanup);
    scrGroupAddTest(group, "Net iterator find IPv4 TCP server", ipv4TcpFindServer, 5, 0);
    scrGroupAddTest(group, "Net iterator find IPv4 TCP client", ipv4TcpFindClient, 5, 0);

    group = scrGroupCreate(ipv6TcpServerSetup, serverCleanup);
    scrGroupAddTest(group, "Net iterator find IPv6 TCP server", ipv6TcpFindServer, 5, 0);
    scrGroupAddTest(group, "Net iterator find IPv6 TCP client", ipv6TcpFindClient, 5, 0);

    group = scrGroupCreate(ipv4UdpServerSetup, serverCleanup);
    scrGroupAddTest(group, "Net iterator find IPv4 UDP server", ipv4UdpFindServer, 5, 0);

    group = scrGroupCreate(ipv6UdpServerSetup, serverCleanup);
    scrGroupAddTest(group, "Net iterator find IPv6 UDP server", ipv6UdpFindServer, 5, 0);

    group = scrGroupCreate(domainServerSetup, domainServerCleanup);
    scrGroupAddTest(group, "Domain socket iterator find server", domainFindServer, 5, 0);

    group = scrGroupCreate(NULL, NULL);
    scrGroupAddTest(group, "Thread iterator", iterateThreads, 5, 0);
    scrGroupAddTest(group, "Map iterator", iterateMaps, 5, 0);

    return scrRun(NULL, NULL);
}
