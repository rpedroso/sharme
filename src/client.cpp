//#include <stdio.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "sharme_config.h"
#include "sharme_ui.h"
#include "screenshot.h"
#include "socket.h"
#include "resize.h"
#include "keyb.h"
#include "mouse.h"
#include "colorspace.h"
#include "common.h"
#include "debug.h"
#include "smoke/smokecodec.h"
#include "fl_thread.h"


static int fast = 0;
static SharmeUI *g_parent = NULL;

static socket_t *client_sock = NULL;
static screenshot_t *ss = NULL;

static SmokeCodecInfo *info = NULL;
static unsigned char* raw_image = NULL;
static unsigned char* yuv420p = NULL;
static unsigned char* outdata = NULL;

static int make_div_by_16(int val)
{
    int m;
    if (!(m=val%16)) return val;
    //return val+(16-m);
    return val-m;
}

static inline void client_screenshot(int width, int height,
                                     int new_w, int new_h)
{
    screenshot_get_image(ss);

    if (width != new_w || height != new_h)
    {
        resample_nearest(ss->data, width, height, 4, raw_image, new_w, new_h);
        rgb2yuv420p(raw_image, yuv420p, new_w, new_h, 3);
    }
    else
    {
        rgb2yuv420p(ss->data, yuv420p, new_w, new_h, 4);
    }
}

static inline int client_sendframe(int yuvsize)
{
    int r;
    int yuvsize_enc;

    yuvsize_enc = yuvsize;

    sharme_tcp_delay(client_sock);
    r = sharme_send(client_sock, (unsigned char*)&yuvsize_enc, 4);
    if (r < 0)
        return r;

    sharme_tcp_nodelay(client_sock);
    r = sharme_send(client_sock, outdata, yuvsize);
    if (r < 0)
        return r;

    return 0;
}


static void *sharme_recv_input(void *arg)
{
    int r;
    char action;
    int pos;
    int key;
    int x, y;
    mouse_init();
    while(true)
    {
        r = sharme_recv(client_sock, (unsigned char*)&action, 1);
        if (r < 0) break;

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
            r = sharme_recv(client_sock, (unsigned char*)&pos, 4);
            if (r < 0) break;
            x = pos>>16;
            y = (pos<<16)>>16;
            mouse_move(x, y);
            break;
        case 'k':
        case 'K':
            //key_flag = (action == 'K') ? KEYEVENTF_KEYUP: 0;
            r = sharme_recv(client_sock, (unsigned char*)&key, 4);
            if (r < 0) break;

            process_key(action, key);
            break;
        }
    }
    pmesg(0, (char*)"client: cleanup keys\n");
    cleanup_keys();
    pmesg(0, (char*)"client: exit thread\n");
}

void* sharme_client2(void *p)
{
    int width;
    int height;
    int new_w;
    int new_h;

    Fl::lock();
    int min_quality = g_parent->sl_quality->value(); //30;
    int max_quality = g_parent->sl_quality->value(); //90;
    Fl::unlock();
    int threshold   = 300;

    char *server = (char *) p;

    int screen_size;

    unsigned int yuvsize;
    unsigned int yuvsize_sav;

    fast = 0;

    client_sock = socket_new(PF_INET, SOCK_STREAM, 0);

    int r = socket_connect(client_sock, server, SHARME_PORT);
    if (r < 0)
    {
        pmesg(0, (char*)"%s: %s\n", "error", "conn refused");
        goto error;
    }

    if (g_parent)
        Fl::awake(connected_cb, g_parent);

    ss = screenshot_new();
    screenshot_get_screen_size(ss, 0, &width, &height);
    screenshot_init(ss, 0, 0, width, height);

    /* advertise the other end about our screen size */
    screen_size = (width<<16) | ((height<<16)>>16);
    r = sharme_send(client_sock, (unsigned char*)&screen_size, 4);
    if (r < 0)
    {
        pmesg(0, (char*)"error: advertising screen size");
        goto error;
    }


    pmesg(1, (char*)"screen size: (%dx%d)\n", width, height);

    new_w = make_div_by_16(width);
    new_h = make_div_by_16(height);

    yuvsize = (new_w * new_h) + ((new_w * new_h) >> 1);
    yuvsize_sav = yuvsize;

    yuv420p = (unsigned char*) calloc(sizeof(unsigned char), yuvsize);
    outdata = (unsigned char*) calloc(sizeof(unsigned char), yuvsize);
    if (new_w != width || new_h != height)
    {
        pmesg(1, (char*)"resample to: (%dx%d)\n", new_w, new_h);
        raw_image = (unsigned char*) calloc(sizeof(unsigned char), new_w*new_h*3);
    }

    smokecodec_encode_new (&info, new_w, new_h, 12, 2);
    smokecodec_set_quality (info, min_quality, max_quality);
    smokecodec_set_threshold (info, threshold);

    Fl_Thread sharme_client_thread2;
    fl_create_thread(sharme_client_thread2, sharme_recv_input, NULL);

    while(true)
    {
        //Fl::lock();
        //int min_quality = g_parent->sl_quality->value(); //30;
        //int max_quality = g_parent->sl_quality->value(); //90;
        //Fl::unlock();
        //smokecodec_set_quality (info, min_quality, max_quality);

        yuvsize = yuvsize_sav;

        client_screenshot(width, height, new_w, new_h);
        smokecodec_encode(info, yuv420p, SMOKECODEC_MOTION_VECTORS, outdata, &yuvsize);
        if ((r=client_sendframe(yuvsize)) < 0)
        {
            pmesg(1, (char*)"error: sendframe (errno: %d\n", r);
            break;
        }

        if (fast)
            usleep(100000);
        else
            usleep(200000);
    }

error:
    if (g_parent)
        Fl::awake(disconnected_cb, g_parent);

    if (ss) screenshot_dealloc(ss);
    if (yuv420p) free(yuv420p);
    if (outdata) free(outdata);
    if (raw_image) free(raw_image);

    socket_close(client_sock);
    socket_del(client_sock);

    if (info) smokecodec_info_free(info);
    free(server);
    pmesg(0, (char*)"sharme_client2: return\n");
}

void sharme_client_stop(void)
{
    if (client_sock)
        socket_shutdown(client_sock, 2);
}

int sharme_client_start(void *parent, char *server)
{
    Fl_Thread sharme_client_thread;

    g_parent = (SharmeUI*) parent;
    connecting_cb(g_parent);

    if (!g_parent->sharme_window->shown())
        sharme_client2((void*)strdup(server));
    else
        fl_create_thread(sharme_client_thread,
                         sharme_client2, (void*)strdup(server));
}
