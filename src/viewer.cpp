#include <stdio.h>
#include <unistd.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

#include <FL/fl_draw.H>
#include <FL/names.h>

#include "debug.h"
#include "resize.h"
#include "socket.h"
#include "colorspace.h"
#include "smoke/smokecodec.h"

#include "sharme_config.h"
#include "common.h"
#include "viewer.h"
#include "sharme_ui.h"
#include "fl_thread.h"


static socket_t *conn = NULL;
static socket_t *sock = NULL;
static int g_width = 0;
static int g_height = 0;

class Viewer;
static Viewer *window = NULL;

#define MAX_WW 2048
#define MAX_HH 1600

class Viewer : public Fl_Double_Window
{
    unsigned char *img_data;
    int img_w;
    int img_h;
    unsigned char rgbout2[MAX_WW*MAX_HH*3];

    void draw()
    {
        pmesg(3, (char*)"Viewer::draw\n");
        //fl_push_clip(0,0,w(),h());
        if (!img_data) return;
        fl_push_no_clip();
        if (img_w != w() || img_h != h())
        {
            resample((unsigned char*)img_data, img_w, img_h, 3, rgbout2, w(), h());
            fl_draw_image((const unsigned char*)rgbout2, 0,0, w(), h(), 3, 0);
        }
        else
            fl_draw_image((const unsigned char*)img_data, 0,0, img_w, img_h, 3, 0);
        fl_pop_clip();
    }

    void resize(int XX,int YY,int WW,int HH) {
        if (WW <= MAX_WW && HH <= MAX_HH)
            Fl_Double_Window::resize(XX,YY,WW,HH);
        //else
        //    Fl_Double_Window::resize(XX,YY,MAX_WW,MAX_HH);
    }

    int handle(int e)
    {
        static int cnt = 0;
        int pos;
        int key;
        char cmd;
        float fw, fh;
        pmesg(3, (char*)"Viewer::handle\n");
        pmesg(9, (char*)"EVENT: %s(%d)\n", fl_eventnames[e], e);

        switch (e)
        {
        case FL_PUSH:
            sharme_tcp_nodelay(conn);
            switch (Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    cmd = 'L';
                    break;
                case FL_MIDDLE_MOUSE:
                    cmd = 'M';
                    break;
                case FL_RIGHT_MOUSE:
                    cmd = 'R';
                    break;
            }
            sharme_send(conn, (unsigned char*)&cmd, 1);
            break;
        case FL_RELEASE:
            sharme_tcp_nodelay(conn);
            switch (Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    cmd = 'l';
                    break;
                case FL_MIDDLE_MOUSE:
                    cmd = 'm';
                    break;
                case FL_RIGHT_MOUSE:
                    cmd = 'r';
                    break;
            }
            sharme_send(conn, (unsigned char*)&cmd, 1);
            break;
        case FL_DRAG:
        case FL_MOVE:
            /* attempt to minimize the number of
               packages transmited */
            cnt++;
            if (!(cnt%2)) {
                usleep(10000);
                cnt=0;
                return 1;
            }
            sharme_tcp_delay(conn);
            cmd = 'c';
            sharme_send(conn, (unsigned char*)&cmd, 1);
            fw = float(g_width) / this->w();
            fh = float(g_height) / this->h();
            pos = ((int)(Fl::event_x()*fw)<<16) | ((((int)(Fl::event_y()*fh))<<16)>>16);
            sharme_tcp_nodelay(conn);
            sharme_send(conn, (unsigned char*)&pos, 4);
            break;
        case 19: // FL_WHEEL
            sharme_tcp_nodelay(conn);
            if(Fl::event_key()-FL_Button == 4) // UP
                cmd = 'W';
            else
                cmd = 'w';
            sharme_send(conn, (unsigned char*)&cmd, 1);
            break;
        case FL_KEYDOWN:
            sharme_tcp_delay(conn);
            key = Fl::event_key();
            cmd = 'k';
            sharme_send(conn, (unsigned char*)&cmd, 1);
            sharme_tcp_nodelay(conn);
            sharme_send(conn, (unsigned char*)&key, 4);
            break;
        case FL_KEYUP:
            sharme_tcp_delay(conn);
            key = Fl::event_key();
            cmd = 'K';
            sharme_send(conn, (unsigned char*)&cmd, 1);
            sharme_tcp_nodelay(conn);
            sharme_send(conn, (unsigned char*)&key, 4);
            break;
        case FL_SHOW:
            set_image(NULL, 0, 0);
            break;
        }
        return 1;
    }

public:
    Viewer(int w,int h) : Fl_Double_Window(w,h) {}

    void set_image(const unsigned char*img_data, int W, int H)
    {
        this->img_w = W;
        this->img_h = H;
        this->img_data = (unsigned char*)img_data;
    }
};

/* messages to main thread */
typedef struct draw_image_args
{
    int width;
    int height;
    unsigned char *data;
} draw_image_args_t;

static void viewer_draw_image_cb(void *p)
{
    pmesg(3, (char*)"viewer_draw_image_cb\n");
    draw_image_args_t *di_arg = (draw_image_args_t*) p;
    window->set_image((const unsigned char*)di_arg->data,
                  di_arg->width, di_arg->height);
    window->redraw();
}

void viewer_disconnected_cb(void *p)
{
    disconnected_cb((SharmeUI*) p);
    Fl::delete_widget(window);
    window = NULL;
}

/* end messages to main thread */

static int viewer_bind(void)
{
    int r;
    int on = 1;
    const char *ip = "0.0.0.0";
    const char *port = SHARME_PORT;
    pmesg(3, (char*)"viewer_bind\n");
    sock = socket_new(PF_INET, SOCK_STREAM, 0);
    if (!sock)
        return 1;
    socket_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));

    pmesg(1, (char*)"binding to %s:%s...\n", ip, port);
    r = socket_bind(sock, ip, port);

    return r;
}

static int viewer_accept(void)
{
    int r;
    pmesg(3, (char*)"viewer_accept\n");
    pmesg(1, (char*)"listen...\n");
    r = socket_listen(sock, 10);
    if (r < 0)
        return r;

    pmesg(1, (char*)"accept...\n");
    conn = socket_accept0(sock);
    if (!conn)
        return 1;

    return 0;
}

static int viewer_recv_protocol_version(void)
{
    int r;
    char proto_version[8];
    char valid_proto_version[] = "SSP0001";
    int proto_version_length = strlen(valid_proto_version);
    char response[] = "Hi,there";

    pmesg(3, (char*)"viewer_recv_protocol_version\n");
    r = sharme_recv(conn, (unsigned char*)proto_version,
                    proto_version_length);
    if (r < 0)
    {
        return -1;
    }
    proto_version[7] = '\0';
    if (strncmp(proto_version, valid_proto_version,
                proto_version_length) != 0)
    {
        return -2;
    }
    r = sharme_send(conn, (unsigned char*)response, strlen(response));
    if (r < 0)
    {
        return -2;
    }
    return 0;
}

static int viewer_recv_remote_screen_size(int *width, int *height)
{
    int r;
    int screen_size;
    pmesg(3, (char*)"viewer_recv_remote_size\n");
    r = sharme_recv(conn, (unsigned char*)&screen_size, 4);
    if (r < 0)
        return -1;

    *width = screen_size>>16;
    *height = (screen_size<<16)>>16;
    return 0;
}

static inline int viewer_recv_frame(unsigned char *videodata, unsigned int *frame_size)
{
    int r;
    pmesg(3, (char*)"viewer_recv_frame\n");
    r = sharme_recv(conn, (unsigned char*)frame_size, 4);
    if (r < 0)
        return -1;

    r = sharme_recv(conn, (unsigned char*)videodata, *frame_size);
    if (r < 0)
        return -1;
    return 0;
}

static inline void viewer_draw_image(unsigned char*data, int width, int height)
{
    draw_image_args_t di_arg;
    di_arg.width = width;
    di_arg.height = height;
    di_arg.data = data;
    Fl::awake(viewer_draw_image_cb, &di_arg);
}

static inline int viewer_codec_parse_header(SmokeCodecInfo *info,
                              unsigned char*data, int size,
                              unsigned int *width, unsigned int *height)
{
    pmesg(3, (char*)"viewer_codec_parse_header\n");
    SmokeCodecFlags flags;
    unsigned int fps_num, fps_denom;
    int ret = smokecodec_parse_header(info, data, size,
                                      &flags, width, height,
                                      &fps_num, &fps_denom);
    return ret;
}

static inline void viewer_window_realize(int width, int height)
{
    pmesg(3, (char*)"viewer_window_realize\n");
    Fl::lock();
    window->size((width > Fl::w() ? Fl::w() : width),
                 (height > Fl::h() ? Fl::h() : height));

    window->position((Fl::w() - window->w())/2,
                     (Fl::h() - window->h())/2);
    window->label("sharme viewer");
    window->show();
    Fl::unlock();
}

static inline void viewer_get_size(int *width, int *height)
{
    pmesg(3, (char*)"viewer_get_size\n");
    Fl::lock();
    *width = window->w();
    *height = window->h();
    Fl::unlock();
}

static void viewer_close_sockets(void)
{
    pmesg(3, (char*)"viewer_close_sockets\n");
    if (conn)
    { 
        socket_close(conn);
        socket_del(conn);
        conn = NULL;
    }
    if (sock)
    {
        socket_close(sock);
        socket_del(sock);
        sock = NULL;
    }
}

static void* viewer_receiver(void* parent)
{
    int r;
    bool first_time = true;
    SmokeCodecInfo *info;
    unsigned int frame_size;
    unsigned char *videodata = 0;

    unsigned char *yuvvideodata = 0;
    unsigned char *rgbvideodata = 0;

    unsigned int frame_width;
    unsigned int frame_height;

    unsigned int last_frame_width = 0;
    unsigned int last_frame_height = 0;

    pmesg(3, (char*)"viewer_receiver\n");

    if (viewer_bind() != 0)
    {
        pmesg(1, (char*)"error create/bind socket\n");
        goto error;
    }

    Fl::awake(ready_cb, parent);

    if (viewer_accept() != 0)
    {
        pmesg(1, (char*)"error listen/accepting connection\n");
        goto error;
    }

    pmesg(1, (char*)"start...\n");
    Fl::awake(connected_cb, parent);

    if ((r = viewer_recv_protocol_version()) < 0)
    {
        if (r == -2)
            pmesg(1, (char*)"incorrect protocol advertisement\n");
        else
            pmesg(1, (char*)"error receiving protocol version\n");
        goto error;
    }
    if (viewer_recv_remote_screen_size(&g_width, &g_height) != 0)
    {
        pmesg(1, (char*)"error recv remote screen size\n");
        goto error;
    }
    pmesg(1, (char*)"remote screen size (%d:%d)\n", g_width, g_height);

    viewer_window_realize(g_width, g_height);

    smokecodec_decode_new(&info);

    videodata = (unsigned char*) malloc(sizeof(unsigned char)
                * g_width*g_height + g_width*g_height/2);
    if (!videodata)
    {
        pmesg(1, (char*)"error out of memory for videodata\n");
        goto error;
    }

    while(true)
    {

        if (viewer_recv_frame(videodata, &frame_size) != 0)
        {
            pmesg(1, (char*)"error recv frame\n");
            break;
        }

        r = viewer_codec_parse_header(info, videodata, frame_size,
                                  &frame_width, &frame_height);

        if ((last_frame_width != frame_width)
             || (last_frame_height != frame_height))
        {
            unsigned long outsize = 0;
            outsize = frame_width * frame_height
                      + frame_width * frame_height / 2;

            yuvvideodata = (unsigned char*) realloc(yuvvideodata,
                           sizeof(char) * outsize);
            if (!yuvvideodata)
            {
                pmesg(1, (char*)"error out of memory\n");
                break;
            }

            rgbvideodata = (unsigned char*) realloc(rgbvideodata, sizeof(char)
                                           * frame_width * frame_height * 3);
            if (!rgbvideodata)
            {
                pmesg(1, (char*)"error out of memory\n");
                break;
            }
        }

        smokecodec_decode(info, (const unsigned char*) videodata,
                          frame_size, yuvvideodata);

        yuv420p2rgb(yuvvideodata, rgbvideodata, frame_width, frame_height, 3);
        viewer_draw_image(rgbvideodata, frame_width, frame_height);

        last_frame_width = frame_width;
        last_frame_height = frame_height;
    }
error:
    Fl::awake(viewer_disconnected_cb, parent);
    if (yuvvideodata) free(yuvvideodata); yuvvideodata = NULL;
    if (rgbvideodata) free(rgbvideodata); rgbvideodata = NULL;
    if (videodata) free(videodata); videodata = NULL;
    viewer_close_sockets();
    pmesg(1, (char*)"exiting viewer thread\n");
}

void sharme_viewer_stop(void)
{
    pmesg(3, (char*)"sharme_viewer_stop\n");
    if (conn)
    {
        socket_shutdown(conn, 2);
        socket_shutdown(sock, 2);
    }
    else
    {
        socket_shutdown(sock, 2);
    }
    /* on win32 this is needed */
    viewer_close_sockets();
}

static void viewer_close(Fl_Widget*, void*)
{
    pmesg(3, (char*)"viewer_close\n");
    sharme_viewer_stop();
}

static Viewer* viewer_create_window(void)
{
    pmesg(3, (char*)"viewer_create_window\n");
    Viewer *w = new Viewer(640, 480);
    w->resizable(w);
    w->end();
    w->callback(viewer_close);
    return w;
}

int sharme_viewer_start(void *parent)
{
    pmesg(3, (char*)"sharme_viewer_start\n");
    window = viewer_create_window();

    Fl::check();

    Fl_Thread sharme_viewer_thread;
    fl_create_thread(sharme_viewer_thread, viewer_receiver, (void*)parent);

    return 0;
}

