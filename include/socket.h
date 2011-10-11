#ifndef __SOKECT_H__
#define __SOKECT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MSWINDOWS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#endif

#define    SAS2SA(x)    ((struct sockaddr *)(x))

typedef struct sockaddr_in sock_addr_t;

/* prototypes */
//int socket_socket(int family, int type, int proto);
//int socket_connect(int fd, const char*host, const char *port);

typedef struct _socket
{
    void *internal;

    int fd;
    int family; // AF_INET or AF_INET6 to force version
    int type;
    int proto;
    struct addrinfo *info;


    //int (*del)(struct _socket *fd);
    //int (*setsockopt)(struct _socket *self, int level, int optname, const void *optval, socklen_t optlen);
    //int (*connect)(struct _socket *self, const char*host, const char *port);
    //int (*bind)(struct _socket *self, const char *host, const char *port);
    //int (*listen)(struct _socket *self, int backlog);
    //int (*recv)(struct _socket *self, void *buf, int len, int flags);
    //int (*send)(struct _socket *self, const void *buf, int len, int flags);
    //int (*sendall)(struct _socket *self, const void *buf, int *len, int flags);
    //int (*close)(struct _socket *self);
} socket_t;

static socket_t* socket_init(int family, int type, int proto);
socket_t* socket_new(int family, int type, int proto);
int socket_del(struct _socket *fd);


int socket_setsockopt(struct _socket *self, int level, int optname, const void *optval, socklen_t optlen);
int socket_connect(struct _socket *self, const char*host, const char *port);
int socket_bind(struct _socket *self, const char *host, const char *port);
int socket_listen(struct _socket *self, int backlog);
struct _socket* socket_accept0(struct _socket *self);
struct _socket* socket_accept(struct _socket *self, struct sockaddr *addr, socklen_t *addrlen);
int socket_recv(struct _socket *self, void *buf, int len, int flags);
int socket_send(struct _socket *self, const void *buf, int len, int flags);
int socket_sendall(struct _socket *self, const void *buf, int *len, int flags);
int socket_close(struct _socket *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOKECT_H__ */

