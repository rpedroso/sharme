#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "arc4.h"
#include "sharme_ui.h"
#include "debug.h"
#include "socket.h"

/* crypto common functions */
static unsigned char* enc_key;
static int key_len;

static inline void sharme_encrypt(unsigned char *data, int size)
{
    struct rc4_key arc4_ct;
    prepare_key((unsigned char*)enc_key, key_len, &arc4_ct);
    rc4(data, size, &arc4_ct);
}

static inline void sharme_decrypt(unsigned char *data, int size)
{
    sharme_encrypt(data, size);
}

void sharme_setup_crypto_key(unsigned char *keycode)
{
    enc_key = keycode;
    key_len = strlen((char*)enc_key);
}

/* random */
char* sharme_random(void)
{
    static char number[7];
    unsigned int n;

    srand(time(NULL) + getpid());		

    for(int i = 0 ; i < 6 ; i++){
        n = rand() % 10;
        number[i] = (char)(n + '0');
    }
    number[6] = '\0';
    return number;
}

/* messages from secondary threads
   to main thread                 */
void disconnected_cb(void *p)
{
    pmesg(3, (char*) "disconnected_cb\n");
    SharmeUI *shui = (SharmeUI*) p;
    shui->disconnected();
}

void connected_cb(void *p)
{
    pmesg(3, (char*) "connected_cb\n");
    SharmeUI *shui = (SharmeUI*) p;
    shui->connected();
}

void ready_cb(void *p)
{
    pmesg(3, (char*)"ready_cb\n");
    SharmeUI *shui = (SharmeUI*) p;
    shui->ready();
}

void connecting_cb(void *p)
{
    pmesg(3, (char*)"connecting_cb\n");
    SharmeUI *shui = (SharmeUI*) p;
    shui->connecting();
}

/*******/
/* TCP */
static inline int tcp_nodelay(socket_t *sock, int on)
{
    return socket_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}
int sharme_tcp_delay(socket_t *sock)
{
    return tcp_nodelay(sock, 0);
}

int sharme_tcp_nodelay(socket_t *sock)
{
    return tcp_nodelay(sock, 1);
}

int sharme_recv(socket_t *sock, unsigned char *buf, int size)
{
    unsigned char* pbuf = buf;
    int r = 0;
    int pos = 0;
    int tot = size;
    while(tot > 0)
    {
        r = socket_recv(sock, pbuf + pos, tot, 0);
        if (r <= 0)
            break;
        tot -= r;
        pos += r;
    }

    if (r <= 0)
        return -1;
    sharme_decrypt(buf, size);
    return 0;
}

int sharme_send(socket_t *sock, unsigned char *buf, int size)
{
    sharme_encrypt(buf, size);
    return socket_sendall(sock, buf, &size, 0);
}
