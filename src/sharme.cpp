#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "arc4.h"

#include "sharme_config.h"
#include "client.h"
#include "viewer.h"
#include "sharme_ui.h"
#include "enums.h"

#include "common.h"

#define PROGNAME "sharme"

int msglevel = 1;
SharmeUI *shui;
static char *cmd_server = NULL;
static int cmd_quality = 40;
static char *cmd_keycode = NULL;
static bool cmd_show = false;

static int helpFlag = 0;

static void viewer(void)
{
    sharme_viewer_start(shui);
}

static void client()
{
    char *server;

    server = strdup(shui->te_server->value());
    if (*server)
    {
        sharme_client_start(shui, server);
    }
    free(server);
}

void start_cb(Fl_Widget *,void *)
{
    sharme_setup_crypto_key(
      (unsigned char*)shui->te_keycode->value());

    //printf("enc_key: %s\n", enc_key);
    if (shui->rb_manage->value())
    {
        viewer();
    }
    else
    {
        client();
    }
}

void mode_cb(Fl_Widget *,void *)
{
    if (shui->rb_share->value())
    {
        shui->gr_props->activate();
    }
    else
    {
        shui->gr_props->deactivate();
    }
}

void exit_cb(Fl_Widget *,void *)
{
    if (shui->state == SHARME_STARTED)
    {
        if (shui->rb_manage->value())
        {
            sharme_viewer_stop();
        }
        else
        {
            sharme_client_stop();
        }
    }
    else
    {
        exit(0);
    }
}

static int arg(int argc, char **argv, int &i)
{
    if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
        helpFlag = 1;
        i += 1;
        return 1;
    }
    if (strcmp("-se", argv[i]) == 0 || strcmp("-server", argv[i]) == 0) {
        if (i < argc-1 && argv[i+1] != 0) {
            cmd_server = argv[i+1];
            i += 2;
            return 2;
        }
    }
    if (strcmp("-q", argv[i]) == 0 || strcmp("-quality", argv[i]) == 0) {
        if (i < argc-1 && argv[i+1] != 0) {
            cmd_quality = atoi(argv[i+1]);
            i += 2;
            return 2;
        }
    }
    if (strcmp("-key", argv[i]) == 0 || strcmp("-keycode", argv[i]) == 0) {
        if (i < argc-1 && argv[i+1] != 0) {
            cmd_keycode = argv[i+1];
            i += 2;
            return 2;
        }
    }
    if (strcmp("-sh", argv[i]) == 0 || strcmp("-show", argv[i]) == 0) {
            cmd_show = true;
            i += 1;
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int i = 1;

    if (Fl::args(argc, argv, i, arg) < argc)
        Fl::fatal("error: unknown option: %s\n"
        "usage: %s [options]\n"
        " -h | --help : print extended help message\n"
        " -se[rver] <ip> : share desktop with <ip>\n"
        " -q[uality] <ip> : image quality\n"
        " -key[code] <key> : key code\n"
        " -sh[ow] : show sharme window at shared desktop side\n"
        " FLTK options\n"
        "%s\n",
        argv[i], argv[0], Fl::help);

    if (helpFlag)
        Fl::fatal("usage: %s [options]\n"
        " -h | --help : print extended help message\n"
        " -se[rver] <ip> : share desktop with <ip>\n"
        " -q[uality] <ip> : image quality\n"
        " -key[code] <key> : key code\n"
        " -sh[ow] : show sharme window at shared desktop side\n"
        " FLTK options:\n"
        "%s\n",
        argv[0], Fl::help);

    Fl::lock();
    shui = new SharmeUI();

    Fl::visual(FL_RGB);

    shui->lb_header->label(SHARME_LABEL_HEADER);

    if (!cmd_server)
    {
        shui->show(argc, argv);
        shui->bt_start->callback(start_cb);
        shui->bt_exit->callback(exit_cb);
        shui->rb_manage->callback(mode_cb);
        shui->rb_share->callback(mode_cb);
        shui->sharme_window->callback(exit_cb);
        shui->te_keycode->value(sharme_random());

        mode_cb(NULL, NULL);
    }
    else
    {
        shui->rb_manage->value(0);
        shui->rb_share->value(1);
        shui->sl_quality->value(cmd_quality);
        shui->te_server->value(cmd_server);
        shui->te_keycode->value(cmd_keycode);

        if (cmd_show)
        {
            shui->bt_start->callback(start_cb);
            shui->bt_exit->callback(exit_cb);
            shui->rb_manage->callback(mode_cb);
            shui->rb_share->callback(mode_cb);
            shui->sharme_window->callback(exit_cb);
            shui->show(argc, argv);
        }

        start_cb(NULL, NULL);
    }

    return Fl::run();
}

