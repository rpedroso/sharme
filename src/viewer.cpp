#include <stdio.h>
#include <string.h>
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

#include "arc4.h"


#ifdef MSWINDOWS
#include <windows.h>
#include <process.h>


typedef unsigned long Fl_Thread;
static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p) {
  return t = (Fl_Thread)_beginthread((void( __cdecl * )( void * ))f, 0, p);
}

#else

#include <unistd.h>
#include <pthread.h>

typedef pthread_t Fl_Thread;
static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p) {
  return pthread_create((pthread_t*)&t, 0, f, p);
}
#endif

extern const unsigned char* enc_key;
extern struct arc4_ctx arc4_ct;

socket_t *conn;
int g_width, g_height;


class Viewer : public Fl_Double_Window
{
    int handle(int e)
    {
        static int cnt=0;
        int pos;
        int key;
        int s;
        int on;
        float fw, fh;
        pmesg(9, (char*)"EVENT: %s(%d)\n", fl_eventnames[e], e);

        switch (e)
        {
        case FL_PUSH:
            on = 1; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            switch (Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    socket_send(conn, "L", 1, 0);
                    break;
                case FL_MIDDLE_MOUSE:
                    socket_send(conn, "M", 1, 0);
                    break;
                case FL_RIGHT_MOUSE:
                    socket_send(conn, "R", 1, 0);
                    break;
            }
            break;
        case FL_RELEASE:
            on = 1; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            switch (Fl::event_button())
            {
                case FL_LEFT_MOUSE:
                    socket_send(conn, "l", 1, 0);
                    break;
                case FL_MIDDLE_MOUSE:
                    socket_send(conn, "m", 1, 0);
                    break;
                case FL_RIGHT_MOUSE:
                    socket_send(conn, "r", 1, 0);
                    break;
            }
            break;
        case FL_DRAG:
        case FL_MOVE:
            cnt++;
            if (!(cnt%2)) {
                usleep(10000);
                cnt=0;
                return 1;
            }
            on = 0; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            socket_send(conn, "c", 1, 0);
            s=4;
            fw = float(g_width) / this->w();
            fh = float(g_height) / this->h();
            //pmesg(9, (char*)"%f:%f   %d:%d   %d:%d\n", fw, fh, g_width, g_height, this->w(), this->h());
            pos = ((int)(Fl::event_x()*fw)<<16) | ((((int)(Fl::event_y()*fh))<<16)>>16);
            on = 1; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            socket_sendall(conn, (char*)&pos, &s, 0);
            break;
        //case 19: // FL_WHEEL
        //    break;
        case FL_KEYDOWN:
            on = 0; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            key = Fl::event_key();
            pmesg(9, (char*)"key: %d\n", key);
            socket_send(conn, "k", 1, 0);
            s=4;
            on = 1; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            socket_sendall(conn, (char*)&key, &s, 0);
            break;
        case FL_KEYUP:
            on = 0; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            key = Fl::event_key();
            socket_send(conn, "K", 1, 0);
            s=4;
            on = 1; socket_setsockopt(conn, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
            socket_sendall(conn, (char*)&key, &s, 0);
            break;
        }
        return 1;
    }
public:
    Viewer(int w,int h) : Fl_Double_Window(w,h) {}
};

void* receiver_func(void* p)
{
    char buf[4096];
    char *pbuf;
    bool first_time=true;
    int r;

    usleep(1000000);

    Viewer *win = (Viewer*) p;

    SmokeCodecInfo *info;
    smokecodec_decode_new (&info);

    pmesg(1, (char*)"start...\n");
    while(true)
    {
        int rest_len;
        char rest[256];

        int len;
        pmesg(8, (char*)"receiving... %d bytes\n", 4);
        r = socket_recv(conn, (char*)&len, 4, 0);
        pmesg(8, (char*)"recv %d bytes\n", r);
        if (r <= 0) break;
        unsigned char *videodata = (unsigned char*) malloc(sizeof(unsigned char) * len);
        unsigned char *pvideodata = videodata;
        unsigned long tot=len;
        r=0; int pos=0;

        while(tot>0)
        {
            pmesg(8, (char*)"receiving... %d bytes\n", tot);
            r = socket_recv(conn, pvideodata+pos, tot, 0);
            pmesg(8, (char*)"recv %d bytes\n", r);
            if (r <= 0) break;
            tot -= r;
            pos += r;
        }
        if (r <= 0)
        {
            free(videodata);
            break;
        }

        arc4_setkey(&arc4_ct, enc_key, strlen((char*)enc_key));
        arc4_decrypt(&arc4_ct, videodata, videodata, len);


        unsigned int width;
        unsigned int height;
        SmokeCodecFlags flags;
        unsigned int fps_num, fps_denom;
        int ret = smokecodec_parse_header (info, videodata, len, &flags, &width, &height, &fps_num, &fps_denom);
        pmesg(9, (char*)"parse_header... %d\n", ret);

        if(first_time) {
            first_time=false;
            win->size( (width>Fl::w()?Fl::w():width)-40 , (height>Fl::h()?Fl::h():height)-40 );
            win->position((Fl::w() - win->w())/2, (Fl::h() - win->h())/2);
        }

        pmesg(9, (char*)"decoding... (%d:%d)(%d:%d)(%d)\n", width, height, fps_num, fps_denom, flags);

        static unsigned int last_width=0;
        static unsigned int last_height=0;
        static unsigned long outsize=0;
        static unsigned char *out=0;
        static unsigned char *rgbout=0;
        static unsigned char *rgbout2=0;

        //float factor = float(height)/width;
        int new_w, new_h;
        Fl::lock();
        new_w = win->w();
        new_h = win->h(); //(int)new_w*factor;
        Fl::unlock();

        if ((last_width != new_w) || last_height != new_h)
        {
            pmesg(9, (char*)"new_w: %d, new_h: %d\n", new_w, new_h);
            pmesg(9, (char*)"last_width: %d, last_height: %d\n", last_width, last_height);
            if (out !=NULL) free(out);
            if (rgbout !=NULL) free(rgbout);
            if (rgbout2 !=NULL) free(rgbout2);
            outsize = width * height + width * height / 2;
            out = (unsigned char*) malloc(sizeof(unsigned char) * outsize);
            rgbout = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);
            rgbout2 = (unsigned char*) malloc(sizeof(unsigned char) * new_w * new_h * 3);
        }

        smokecodec_decode (info, (const unsigned char*) videodata, len, out);
        pmesg(9, (char*)"yuv4202rgb %p\n", rgbout);
        yuv420p2rgb(out, rgbout, width, height, 3);

        pmesg(9, (char*)"resample\n");
        resample(rgbout, width, height, 3, rgbout2, new_w, new_h);

        pmesg(9, (char*)"draw\n");
        Fl::lock();
        fl_push_no_clip();
        fl_draw_image((const unsigned char*)rgbout2, 0,0, new_w, new_h, 3, 0); //win->w(), win->h());
        fl_pop_clip();
        Fl::unlock();

        free(videodata);

        last_width = new_w;
        last_height = new_h;
        //usleep(125000);
    }
    pmesg(1, (char*)"exiting viewer thread\n");
}

int viewer()
{
    Fl_Color fg = FL_BLACK;
    Fl_Color bg = FL_WHITE;

    int r, yes=1;
    const char *ip = "0.0.0.0";
    const char *port="8000";
    socket_t *sock = socket_new(PF_INET, SOCK_STREAM, 0);
    socket_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int));

    pmesg(1, (char*)"binding to %s:%s...\n", ip, port);
    r = socket_bind(sock, ip, port);
    pmesg(1, (char*)"binded to %s:%s...(%d)\n", ip, port, r);

    socket_listen(sock, 10);
    pmesg(1, (char*)"listen...(%d)\n", r);

    pmesg(1, (char*)"accept...\n");
    conn = socket_accept0(sock);

    int screen_size;
    r = socket_recv(conn, (char*)&screen_size, sizeof(int), 0);
    g_width = screen_size>>16;
    g_height = (screen_size<<16)>>16;
    pmesg(1, (char*)"remote screen size (%d:%d)\n", g_width, g_height);




  Viewer window(640,480);
  window.resizable(window);

  window.end();
  window.show();
  fl_cursor(Fl_Cursor(0),fg,bg);
  //fl_cursor(FL_CURSOR_NONE,fg,bg);

  Fl_Thread receiver_thread;

  Fl::unlock();
  Fl::lock();
  fl_create_thread(receiver_thread, receiver_func, (void*)&window);

  window.make_current();
  int ret = Fl::run();

  pmesg(1, (char*)"closing socket...\n");
  socket_close(conn);
  socket_close(sock);
  return ret;
}

