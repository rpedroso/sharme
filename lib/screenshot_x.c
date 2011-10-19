#include <stdio.h>
//#include <jpeglib.h>
#include <stdlib.h>
#include "screenshot.h"
#include "screenshot_x.h"

#define HAVE_XSHM
//#undef HAVE_XSHM

#ifdef HAVE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

void
ximage_free (XImage* ximage)
{
  /* make sure it is not recycled */
  ximage->width = -1;
  ximage->height = -1;
  free(ximage);
}

Context* display_open(const char *display_name)
{
  Context *xcontext;
  xcontext = (Context*) malloc(sizeof(Context));

  xcontext->disp = XOpenDisplay(display_name);
  printf("opened display %p\n", xcontext->disp);
  if (!xcontext->disp) {
    free (xcontext);
    return NULL;
  } 
  xcontext->screen = DefaultScreenOfDisplay (xcontext->disp);
  xcontext->screen_num = DefaultScreen (xcontext->disp);
  xcontext->visual = DefaultVisual (xcontext->disp, xcontext->screen_num);
  xcontext->root = DefaultRootWindow (xcontext->disp);
  xcontext->white = XWhitePixel (xcontext->disp, xcontext->screen_num);
  xcontext->black = XBlackPixel (xcontext->disp, xcontext->screen_num);
  xcontext->depth = DefaultDepthOfScreen (xcontext->screen);
  printf("Depth %d\n", xcontext->depth); 
  xcontext->width = DisplayWidth (xcontext->disp, xcontext->screen_num);
  xcontext->height = DisplayHeight (xcontext->disp, xcontext->screen_num);
          
  xcontext->widthmm = DisplayWidthMM (xcontext->disp, xcontext->screen_num);
  xcontext->heightmm = DisplayHeightMM (xcontext->disp, xcontext->screen_num);

  printf("bitorder %d\n", ImageByteOrder (xcontext->disp));
  return xcontext;
}


#ifdef HAVE_XSHM
  XShmSegmentInfo SHMInfo;
#endif

XImage* ximage_new (Context *xcontext, int width, int height)
{
  XImage *ximage = NULL;
  int succeeded = 0;
  int size = 0;

  int (*handler) (Display *, XErrorEvent *);

  ximage = (XImage*) malloc (sizeof(XImage));

  ximage->width = width;
  ximage->height = height;

#ifdef HAVE_XSHM

    ximage = XShmCreateImage (xcontext->disp,
        xcontext->visual, xcontext->depth,
        ZPixmap, NULL, &SHMInfo, ximage->width, ximage->height);
    if (!ximage) {
      printf("!ximage\n");
      goto beach; 
    }
    
    ///* we have to use the returned bytes_per_line for our shm size */
    size = ximage->bytes_per_line * ximage->height;
    printf("size %d\n", size);
    SHMInfo.shmid = shmget (IPC_PRIVATE, size, IPC_CREAT | 0777);
    if (SHMInfo.shmid == -1) {
      printf("shmid == -1\n");
      goto beach;
    }
    
    SHMInfo.shmaddr = shmat (SHMInfo.shmid, 0, 0);
    if (SHMInfo.shmaddr == ((void *) -1)) {
      printf("shaddr == -1\n");
      goto beach;
    }
    SHMInfo.readOnly = 0;

    ///* Delete the SHM segment. It will actually go away automatically
    // * when we detach now */
    shmctl (SHMInfo.shmid, IPC_RMID, 0);

    ximage->data = SHMInfo.shmaddr;

    if (XShmAttach (xcontext->disp, &SHMInfo) == 0) {
      printf("Falied to attach\n");
      goto beach;
    }

    XSync (xcontext->disp, 0);
#else /* HAVE_XSHM */
//    ximage = XCreateImage (xcontext->disp,
//        xcontext->visual,
//        xcontext->depth,
//        ZPixmap, 0, NULL, ximage->width, ximage->height, xcontext->bpp, 0);
//    if (!ximage)
//      goto beach;
//
//    /* we have to use the returned bytes_per_line for our image size */
//    ximage->size = ximage->ximage->bytes_per_line * ximage->ximage->height;
//    ximage->data = g_malloc (ximage->size);
//
//    XSync (xcontext->disp, FALSE);
#endif /* HAVE_XSHM */
  succeeded = 1;

beach:
  pmesg(9, "succeeded %d\n", succeeded);

  if (!succeeded) {
    ximage_free (ximage);
    ximage = NULL;
  }

  return ximage;
}

#define DEPTH8    0x00e0e0c0 
#define DEPTH12    0x00f0f0f0 
#define DEPTH16    0x00f1f1f1 
#define DEPTH24    0x00ffffff 
#define DEPTH32    0xffffffff 

typedef struct _private
{
    Context *context;
    XImage *image;
    int startx;
    int starty;
    int width;
    int height;
} private_t;

void screenshot_init(screenshot_t *self, int startx, int starty, int width, int height)
{
    pmesg(9, "screenshot_init\n");
    private_t *priv = (private_t*)self->priv;
    priv->startx = startx;
    priv->starty = starty;
    priv->width = width;
    priv->height = height;

#ifdef HAVE_XSHM
    //priv->image = XShmCreateImage (disp, visual, depth, ZPixmap, NULL, &SHMInfo, width, height);
    //priv->image->data = calloc (size,1);
    priv->image = ximage_new (priv->context, width, height);
#else

    priv->image = XCreateImage (priv->context->disp,
        priv->context->visual,
        priv->context->depth,
        ZPixmap, 0, NULL, priv->width, priv->height, 32, 0);
    long size = priv->image->bytes_per_line * priv->image->height;
    priv->image->data = calloc (size,1);
#endif

}

inline void screenshot_get_image(screenshot_t *self) //, int startx, int starty, int width, int height)
{
    pmesg(9, "screenshot_get_image\n");
    XImage *ximage;
    private_t *priv;

    priv = (private_t*)self->priv;

    int startx = priv->startx;
    int starty = priv->starty;
    int width = priv->width;
    int height = priv->height;

#ifdef HAVE_XSHM

    pmesg(9, "getting screenshot %d\n", priv->image==NULL?0:1);
    int r = XShmGetImage (priv->context->disp, priv->context->root, priv->image, startx, starty, AllPlanes);
    pmesg(9, "ret %d\n", r);
    self->data = priv->image->data;

#else
    //priv->image = XGetImage (priv->context->disp, priv->context->root, startx, starty, width, height, DEPTH32, ZPixmap);
    XGetSubImage (priv->context->disp, priv->context->root,
            startx, starty, width, height, AllPlanes, ZPixmap, priv->image, 0, 0);

    self->data = priv->image->data;
    //printf("bpp %d depth %d\n", ximage->bits_per_pixel, ximage->depth);
    //printf("bpl %d\n", ximage->bytes_per_line);
#endif /* HAVE_XSHM */

    //write_jpeg_file(ximage->data, width, height, "zz.jpg" );
}


void screenshot_get_screen_size(screenshot_t *self, int index, int *width, int *height)
{
    pmesg(9, "get_screen_size\n");
    private_t *priv = (private_t*)self->priv;
    *width = priv->context->width;
    *height = priv->context->height;
}


void screenshot_free_image(screenshot_t *self)
{
    pmesg(9, "screenshot_free_image\n");
    private_t *priv = (private_t*)self->priv;
    if (priv->image)
        XDestroyImage(priv->image);
}

void screenshot_dealloc(screenshot_t *self)
{
    pmesg(9, "screenshot_dealloc\n");
    if (!self) return;

    if (self->priv)
    {
        screenshot_free_image(self);
        free(self->priv);
    }
    free(self);
}

screenshot_t* screenshot_new()
{
    pmesg(9, "screenshot_new\n");
    screenshot_t *ss = (screenshot_t*) calloc(sizeof(screenshot_t), 1);

    ss->priv = (private_t*)calloc(sizeof(private_t), 1);
    private_t *priv = (private_t*)ss->priv;

    priv->context = display_open(NULL);

    //ss->init = screenshot_init;
    //ss->get_image = screenshot_get_image;
    //ss->get_screen_size = screenshot_get_screen_size;
    //ss->free_image = screenshot_free_image;
    //ss->dealloc = screenshot_dealloc;

    return ss;
}
