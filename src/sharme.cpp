#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>

#include <stdlib.h>
#include <string.h>

//#include "debug.h"
#include "arc4.h"
#include "client.h"
#include "viewer.h"

#define PROGNAME "Sharme"

int msglevel = 1;

const unsigned char *enc_key = (const unsigned char*) "alibaba";
struct arc4_ctx arc4_ct;

typedef struct {
    Fl_Window *win;
    Fl_Input *inp;
} objs;

void viewer_cb(Fl_Widget *b,void *p)
{
    Fl_Window *w = (Fl_Window*)p;
    w->hide();
    Fl::wait();
    viewer();
    w->show();
}

void client_cb(Fl_Widget *b,void *p)
{
    char *ip;
    objs *o = (objs*) p;

    Fl_Window *window = o->win;
    Fl_Input *input = o->inp;

    ip = strdup(input->value());
    if (*ip)
    {
        b->deactivate();
        window->hide();
        Fl::wait();
        sharme_client(ip);
        window->show();
        b->activate();
    }
    free(ip);
}

void exit_cb(Fl_Widget *,void *)
{
    exit(0);
}

int main(int argc, char *argv[])
{
    objs o;

    if (argv[1] && *argv[1])
    {
        sharme_client(argv[1]);
        exit(0);
    }

    Fl::args(argc, argv);
    Fl::get_system_colors();
    Fl::visual(FL_RGB);

    Fl_Window window(320, 85);
    window.label(PROGNAME);

    int y = 10;
    Fl_Input input(30, y, 300, 30, "IP:");
    y += 35;
    input.tooltip("");

    o.win = &window;
    o.inp = &input;

    int x = 9;
    Fl_Button b_ok(x, y, 110, 25, "&Share Screen");
    x += 120;
    b_ok.callback(client_cb, (void*)&o);
    b_ok.tooltip("");

    Fl_Button b_server(x, y, 110, 25, "&Remote Screen"); x+=120;
    b_server.callback(viewer_cb, (void*)&window);
    b_server.tooltip("");

    Fl_Button *b = new Fl_Button(x, y, 110, 25, "&Cancel");
    x += 120;
    y += 25;
    b->callback(exit_cb);
    b->tooltip("");

    window.size(380, y+10);
    window.end();
    window.show(argc, argv);

    Fl::run();
}

