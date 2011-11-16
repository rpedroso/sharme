#include <stdio.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>

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


static int keep_running = 1;
static int fast = 0;
static SharmeUI *g_parent = NULL;

static socket_t *client_sock = NULL;
static screenshot_t *ss = NULL;

static SmokeCodecInfo *info = NULL;
static unsigned char* raw_image = NULL;
static unsigned char* yuv420p = NULL;
//static unsigned char* outdata = NULL;

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

    r = sharme_tcp_delay(client_sock);
    if (r < 0)
        return r;
    r = sharme_send(client_sock, (unsigned char*)&yuvsize_enc, 4);
    if (r < 0)
        return r;

    r = sharme_tcp_nodelay(client_sock);
    if (r < 0)
        return r;
    r = sharme_send(client_sock, ss->data, yuvsize);
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
    int cnt = 0;
    while(keep_running)
    {
        cnt++;
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
            if (cnt == 2)
            {
                x = pos>>16;
                y = (pos<<16)>>16;
                mouse_move(x, y);
            }
            break;
        case 'W':
            mouse_wheel(4);
            break;
        case 'w':
            mouse_wheel(5);
            break;
        case 'k':
        case 'K':
            //key_flag = (action == 'K') ? KEYEVENTF_KEYUP: 0;
            r = sharme_recv(client_sock, (unsigned char*)&key, 4);
            if (r < 0) break;

            process_key(action, key);
            break;
        }
        if (cnt == 2)
        {
            Fl::awake();
            cnt=0;
        }
    }
    pmesg(9, (char*)"client: cleanup keys\n");
    cleanup_keys();
    pmesg(9, (char*)"client: exit thread\n");
    keep_running = 0;
}

void* sharme_client2(void *p)
{
    int width;
    int height;
    int new_w;
    int new_h;

    int min_quality = g_parent->sl_quality->value();
    int max_quality = g_parent->sl_quality->value();
    int threshold   = 1;

    char *server = (char *) p;

    int screen_size;

    unsigned int yuvsize;
    unsigned int yuvsize_sav;

    double time;

    fast = 0;

    client_sock = socket_new(PF_INET, SOCK_STREAM, 0);

    int r = socket_connect(client_sock, server, SHARME_PORT);
    if (r < 0)
    {
        pmesg(0, (char*)"%s: %s\n", "error", "conn refused");
        fl_alert("Could not connect");
        goto error;
    }

    if (g_parent)
        //Fl::awake(connected_cb, g_parent);
        connected_cb(g_parent);

    /* trivial handshake */
    char proto_version[8];
    snprintf(proto_version, 8, "SSP0001");
    pmesg(9, (char*) "%s\n", proto_version);
    r = sharme_send(client_sock, (unsigned char*)proto_version, 7);
    if (r < 0)
    {
        pmesg(0, (char*)"error: advertising proto version\n");
        fl_alert("Could not start sharing session.\nVerify your network");
        goto error;
    }

    {
        char valid_response[] = "Hi,there";
        int response_size = strlen(valid_response);
        char response[response_size];
        r = sharme_recv(client_sock, (unsigned char*)response, strlen(valid_response));
        if (r < 0)
        {
            pmesg(0, (char*)"error: receiving handshake message\n");
            fl_alert("Could not start sharing session.\nCould be a wrong key code");
            goto error;
        }
        response[response_size] = '\0';
        pmesg(3, (char*)"RESPONSE: %s\n", response);
        if (strncmp(response, valid_response, response_size))
        {
            pmesg(0, (char*)"error: received handshake message incorrect\n");
            fl_alert("Could not start sharing session.\nCould be a wrong key code");
        }
    }

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
    //outdata = (unsigned char*) calloc(sizeof(unsigned char), yuvsize);
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

    static int skip = 0;
    while(keep_running)
    {
        yuvsize = yuvsize_sav;

        if (skip < 4) {
        client_screenshot(width, height, new_w, new_h);
        smokecodec_encode(info, yuv420p, SMOKECODEC_MOTION_VECTORS, ss->data, &yuvsize);
        if (yuvsize > 18)
        {
            if ((r=client_sendframe(yuvsize)) < 0)
            {
                pmesg(1, (char*)"error: sendframe (errno: %d)\n", r);
                break;
            }
        }
        }
        if (skip > 4) skip = 0;

        time = Fl::wait(.5);
        if (time)
        {
            if (yuvsize > 40000)
                skip++;
            if (yuvsize > 80000)
                skip++;
            usleep(125000);
            min_quality = g_parent->sl_quality->value();
            max_quality = g_parent->sl_quality->value();
            smokecodec_set_quality (info, min_quality, max_quality);
        }
    }

error:
    if (g_parent)
        //Fl::awake(disconnected_cb, g_parent);
        disconnected_cb(g_parent);

    pmesg(9, (char*)"SS: %p\n", ss);
    if (ss) { screenshot_dealloc(ss); ss = NULL; }
    pmesg(9, (char*)"YUV: %p\n", yuv420p);
    if (yuv420p) { free(yuv420p); yuv420p = NULL; }
    //if (outdata) free(outdata);
    pmesg(9, (char*)"RAW: %p\n", raw_image);
    if (raw_image) { free(raw_image); raw_image = NULL; }

    pmesg(9, (char*)"shut: %p\n", client_sock);
    socket_shutdown(client_sock, 2);
    pmesg(9, (char*)"close: %p\n", client_sock);
    socket_close(client_sock);
    pmesg(9, (char*)"del: %p\n", client_sock);
    socket_del(client_sock); { client_sock = NULL; }

    pmesg(0, (char*)"info: %p\n", info);
    if (info) { smokecodec_info_free(info); info = NULL; }
    pmesg(0, (char*)"sharme_client2: return\n");
}

void sharme_client_stop(void)
{
    if (client_sock)
    {
        keep_running = 0;
    }
}

int sharme_client_start(void *parent, char *server)
{
    //Fl_Thread sharme_client_thread;

    keep_running = 1;
    g_parent = (SharmeUI*) parent;
    connecting_cb(g_parent);

    sharme_client2((void*)server);
    pmesg(0, (char*)"sharme_client_start: return\n");
}
