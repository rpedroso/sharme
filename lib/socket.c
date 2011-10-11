#include <stdio.h>
#include "socket.h"


int socket_close(socket_t *self)
{
#ifdef MSWINDOWS
    closesocket(self->fd);
    WSACleanup();
    self->fd = -1;
#else
    close(self->fd);
#endif
}

int socket_del(socket_t *self)
{
    free(self);
}

int socket_setsockopt(socket_t *self, int level, int optname, const void *optval, socklen_t optlen)
{
    return setsockopt(self->fd, level, optname, optval, optlen);
}

int socket_connect(socket_t *self, const char*host, const char *port)
{
    int r;
    struct addrinfo hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = self->family;
    hints.ai_socktype = self->type;
    hints.ai_protocol = self->proto;

    if ((r = getaddrinfo(host, port, &hints, &self->info)) != 0) {
        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        freeaddrinfo(self->info);
        return -2;
    }

    r = connect(self->fd, self->info->ai_addr, self->info->ai_addrlen);
    freeaddrinfo(self->info);
    return r;
}

int socket_bind(socket_t *self, const char *host, const char *port)
{
    int r;
    struct addrinfo hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = self->family;
    hints.ai_socktype = self->type;
    hints.ai_protocol = self->proto;

    if ((r = getaddrinfo(host, port, &hints, &self->info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
        return -1;
    }

    r = bind(self->fd, self->info->ai_addr, self->info->ai_addrlen);
    freeaddrinfo(self->info);
    return r;
}

int socket_listen(socket_t *self, int backlog)
{
    int r = listen(self->fd, backlog);
    return r;
}

socket_t* socket_accept0(socket_t *self)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    return socket_accept(self, (struct sockaddr *)&their_addr, &addr_size);
}

socket_t* socket_accept(socket_t *self, struct sockaddr *addr, socklen_t *addrlen)
{
    //int new_fd = accept(self->fd, (struct sockaddr *)addr, addrlen);
    int new_fd = accept(self->fd, NULL, NULL);

    socket_t *new_self = socket_init(self->family, self->type, self->proto);
    new_self->fd = new_fd;
    return new_self;
}

int socket_recv(socket_t *self, void *buf, int len, int flags)
{
    return recv(self->fd, (char*)buf, len, flags);
}

int socket_send(socket_t *self, const void *buf, int len, int flags)
{
    return send(self->fd, (const char*)buf, len, flags);
}

int socket_sendall(socket_t *self, const void *buf, int *len, int flags)
{
    // how many bytes we've sent
    int total = 0;
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(self->fd, ((const char*)buf)+total, bytesleft, flags);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

static socket_t* socket_init(int family, int type, int proto)
{
#ifdef MSWINDOWS
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        return NULL;
    }
#endif
    socket_t *self = (socket_t*) calloc(1, sizeof(socket_t));
    if (self == NULL)
        return NULL;

    //memset(self, 0, sizeof(socket_t));

    //self->del = socket_del;
    //self->setsockopt = socket_setsockopt;
    //self->connect = socket_connect;
    //self->bind = socket_bind;
    //self->listen = socket_listen;
    //self->accept0 = socket_accept0;
    //self->accept = socket_accept;
    //self->recv = socket_recv;
    //self->send = socket_send;
    //self->sendall = socket_sendall;
    //self->close = socket_close;

    self->family = family;
    self->type = type;
    self->proto = proto;

    return self;
}

socket_t* socket_new(int family, int type, int proto)
{
    socket_t *self = socket_init(family, type, proto);

    self->fd = socket(family, type, proto);

    return self;
}
