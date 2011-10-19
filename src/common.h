#ifndef sharme_common_h
#define sharme_common_h
#include "socket.h"

void sharme_encrypt(unsigned char *data, int size);
void sharme_decrypt(unsigned char *data, int size);
void sharme_setup_crypto_key(unsigned char *keycode);

char* sharme_random(void);

void disconnected_cb(void *p);
void connected_cb(void *p);
void ready_cb(void *p);
void connecting_cb(void *p);

void sharme_tcp_delay(socket_t *sock);
void sharme_tcp_nodelay(socket_t *sock);
int sharme_recv(socket_t *sock, unsigned char *buf, int size);
int sharme_send(socket_t *sock, unsigned char *buf, int size);

#endif
