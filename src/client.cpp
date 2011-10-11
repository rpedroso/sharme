#include <pthread.h>
#include <unistd.h>

#include "screenshot.h"
#include "socket.h"
#include "resize.h"
#include "keyb.h"
#include "mouse.h"
#include "colorspace.h"
#include "arc4.h"
#include "debug.h"
#include "smoke/smokecodec.h"


static int fast;

extern const unsigned char* enc_key;
extern struct arc4_ctx arc4_ct;


int make_div_by_16(int val)
{
    int m;
    if (!(m=val%16)) return val;
    //return val+(16-m);
    return val-m;
}


socket_t *sock;
screenshot_t *ss;

SmokeCodecInfo *info;
static unsigned char* raw_image;
static unsigned char* yuv420p;
static unsigned char* outdata;

static inline void sharme_screenshot(int width, int height, int new_w, int new_h, unsigned int *yuvsize)
{
    SmokeCodecFlags flags;

    screenshot_get_image(ss);

    if (width != new_w || height != new_h)
    {
      resample_nearest(ss->data, width, height, 4, raw_image, new_w, new_h);
      //resample(ss->data, width, height, 4, raw_image, new_w, new_h);
      rgb2yuv420p(raw_image, yuv420p, new_w, new_h, 3);
    }
    else
    {
        //memcpy(raw_image, ss->data, width*height*4);
        raw_image = ss->data;
        rgb2yuv420p(raw_image, yuv420p, new_w, new_h, 4);
    }

    flags = SMOKECODEC_MOTION_VECTORS;
    smokecodec_encode(info, yuv420p, flags, outdata, yuvsize);

}

static inline int sharme_sendframe(int yuvsize)
{
    int r;
    int s = 4;

    int on = 0;
    socket_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    //arc4_setkey(&arc4_ct, enc_key, strlen(enc_key));
    //arc4_encrypt(&arc4_ct, (char*)yuvsize, yuvsize, s);

    r = socket_sendall(sock, (char*)&yuvsize, &s, 0);
    if (r < 0)
    {
        pmesg(0, (char*)"%s: %s\n", "erro", "conn close");
        return(-1);
    }

    on = 1;
    socket_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));

    arc4_setkey(&arc4_ct, enc_key, strlen((char*)enc_key));
    arc4_encrypt(&arc4_ct, outdata, outdata, yuvsize);

    r = socket_sendall(sock, outdata, &yuvsize, 0);
    if (r < 0)
    {
        pmesg(0, (char*)"%s: %s\n", "erro", "conn close");
        return(-1);
    }
    return 0;
}


void *sharme_recv_input(void *arg)
{
    int r;
    char action;
    char pbuf[1]={0};
    int pos;
    int key;
    int x, y;
    mouse_init();
    while(1)
    {
        r = socket_recv(sock, (char*)&action, 1, 0);
        if (r <= 0) break;

        switch (action)
        {
        case 'L':
            fast = 1;
            mouse_left_down();
            break;
        case 'M':
            break;
        case 'R':
            mouse_right_down();
            break;
        case 'l':
            fast = 0;
            mouse_left_up();
            break;
        case 'r':
            mouse_right_up();
            break;
        case 'c':
            r = socket_recv(sock, (char*)&pos, 4, 0);
            if (r <= 0) break;
            x = pos>>16;
            y = (pos<<16)>>16;
            mouse_move(x, y);
            break;
        case 'k':
        case 'K':
            //key_flag = (action == 'K') ? KEYEVENTF_KEYUP: 0;
            r = socket_recv(sock, (char*)&key, 4, 0);
            if (r <= 0) break;

            process_key(action, key);
            break;
        }
    }
    pmesg(0, (char*)"client: exit thread\n");
}

int sharme_client(char *ip)
{
    int width;
    int height;
    int new_w;
    int new_h;

    int min_quality = 30;
    int max_quality = 90;
    int threshold = 300;

    fast = 0;

    pthread_t thread_ID;
    void *exit_status;

    sock = socket_new(PF_INET, SOCK_STREAM, 0);

    int r = socket_connect(sock, ip, "8000");
    if (r < 0)
    {
        pmesg(0, (char*)"%s: %s\n", "erro", "conn refused");
        return(-1);
    }

    pthread_create(&thread_ID ,NULL, sharme_recv_input, NULL);

    ss = screenshot_new();
    screenshot_get_screen_size(ss, 0, &width, &height);
    screenshot_init(ss, 0, 0, width, height);

    new_w = make_div_by_16(width);
    new_h = make_div_by_16(height);

    int screen_size = (width<<16) | ((height<<16)>>16);
    int s = sizeof(int);
    r = socket_sendall(sock, (char*)&screen_size, &s, 0);

    pmesg(1, (char*)"screen size: (%dx%d)\n", width, height);
    pmesg(1, (char*)"rescaling to: (%dx%d)\n", new_w, new_h);

    smokecodec_encode_new (&info, new_w, new_h, 12, 2);
    smokecodec_set_quality (info, min_quality, max_quality);
    smokecodec_set_threshold (info, threshold);

    unsigned int yuvsize = (new_w * new_h) + ((new_w * new_h) >> 1);
    unsigned int yuvsize_sav = yuvsize;

    yuv420p = (unsigned char*) calloc(sizeof(unsigned char), yuvsize);
    outdata = (unsigned char*) calloc(sizeof(unsigned char), yuvsize);
    raw_image = (unsigned char*) calloc(sizeof(unsigned char), new_w*new_h*3);

    while(true)
    {
        yuvsize = yuvsize_sav;

        sharme_screenshot(width, height, new_w, new_h, &yuvsize);
        if (sharme_sendframe(yuvsize)<0) break;

        if (fast)
            usleep(100000);
        else
            usleep(200000);
    }

    pthread_join(thread_ID, &exit_status);

    cleanup_keys();

    screenshot_dealloc(ss);
    free(yuv420p);
    free(outdata);
    free(raw_image);

    socket_close(sock);
    socket_del(sock);

    smokecodec_info_free(info);
    return 0;
}
