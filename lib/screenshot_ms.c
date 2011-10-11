#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <debug.h>

typedef struct _private
{
    HDC winDC;
    HDC memDC;
    BYTE *dibMem;
    int startx;
    int starty;
    int width;
    int height;
    HBITMAP hBitmap;
} private_t;

#define BITSPERPIXEL 32
void screenshot_init(screenshot_t *self, int startx, int starty, int width, int height)
{
printf("init\n");
    HWND capture;

    BITMAPINFO info;

    ZeroMemory(&info,sizeof(BITMAPINFO));

    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biBitCount=BITSPERPIXEL;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biWidth=width;
    info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;
    info.bmiHeader.biSizeImage=abs(info.bmiHeader.biHeight)*info.bmiHeader.biWidth*info.bmiHeader.biBitCount/8;

    private_t *priv = (private_t*) self->priv;
    priv->startx = startx;
    priv->starty = starty;
    priv->width = width;
    priv->height = height;

    capture = GetDesktopWindow();
    priv->winDC = GetDC(capture);
    priv->memDC = CreateCompatibleDC(priv->winDC);

    // make a bmp in memory to store the capture in
    priv->hBitmap = CreateDIBSection (priv->memDC, &info, DIB_RGB_COLORS, (void **) &priv->dibMem, 0, 0);
    // join em up
    SelectObject(priv->memDC, priv->hBitmap);
    DeleteObject(capture);
    DeleteObject(&info);
}

inline void _capture_cursor(HDC memDC)
{
   //HBITMAP bmp;
   //HICON hicon;
   CURSORINFO ci;
   ICONINFO icInfo;
   int x, y;
   ci.cbSize = sizeof(CURSORINFO);
   if(GetCursorInfo(&ci))
   {
       //fprintf(stderr, "GetCursorInfo\n");
       if (ci.flags & CURSOR_SHOWING)
       { 
           //fprintf(stderr, "CURSOR_SHOING\n");
           //hicon = CopyIcon(ci.hCursor);
           if(GetIconInfo(ci.hCursor, &icInfo))
           {
               //fprintf(stderr, "GetIconInfo\n");
               x = ci.ptScreenPos.x - ((int)icInfo.xHotspot);
               y = ci.ptScreenPos.y - ((int)icInfo.yHotspot);
   //            Icon ic = Icon.FromHandle(hicon);
   //            bmp = ic.ToBitmap(); 

               //fprintf(stderr, "DrawIcon (%dx%d)\n", x, y);
               //DrawIcon(memDC, x, y, hicon);
               DrawIconEx (memDC,
                   ci.ptScreenPos.x - icInfo.xHotspot,
                   ci.ptScreenPos.y - icInfo.yHotspot, ci.hCursor, 0, 0,
                   0, NULL, DI_DEFAULTSIZE | DI_NORMAL | DI_COMPAT);

   //            return bmp;
           }
       }
   }
}

inline void screenshot_get_cursor(screenshot_t *self, int *posx, int *posy, unsigned char *bmp)
{
    private_t *priv = (private_t*) self->priv;

   //HBITMAP bmp;
   HICON hicon;
   CURSORINFO ci;
   ICONINFO icInfo;
   int x, y;
   ci.cbSize = sizeof(CURSORINFO);
   if(GetCursorInfo(&ci))
   {
       //fprintf(stderr, "GetCursorInfo\n");
       if (ci.flags == CURSOR_SHOWING)
       {
           //fprintf(stderr, "CURSOR_SHOING\n");
           hicon = CopyIcon(ci.hCursor);
           if(GetIconInfo(hicon, &icInfo))
           {
               //fprintf(stderr, "GetIconInfo\n");
               x = ci.ptScreenPos.x - ((int)icInfo.xHotspot);
               y = ci.ptScreenPos.y - ((int)icInfo.yHotspot);
               //Icon ic = Icon.FromHandle(hicon);
               //bmp = ic.ToBitmap(); 

               //fprintf(stderr, "DrawIcon (%dx%d)\n", x, y);
               //DrawIcon(memDC, x, y, hicon);

   //            return bmp;
               bmp = (unsigned char*)icInfo.hbmColor;
           }
       }
   }

}

inline void screenshot_get_image(screenshot_t *self) //, int startx, int starty, int width, int height)
{
    private_t *priv = (private_t*) self->priv;

    // copy from the screen to my bitmap
    BitBlt(priv->memDC, 0, 0, priv->width, priv->height,
           priv->winDC, priv->startx, priv->starty, SRCCOPY|CAPTUREBLT);
    _capture_cursor(priv->memDC);

    self->data = priv->dibMem;
}

void screenshot_get_screen_size(screenshot_t *self, int index, int *width, int *height)
{
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);

}

void screenshot_free_image(screenshot_t *self)
{
}

void screenshot_dealloc(screenshot_t *self)
{
    private_t *priv = (private_t*) self->priv;
    DeleteObject(self->data);
    if (self->priv)
    {
        pmesg(9, "free priv->hBitmap\n");
        DeleteObject(priv->hBitmap);
        pmesg(9, "free priv->dibMem\n");
        DeleteObject(priv->winDC);
        DeleteObject(priv->memDC);
        pmesg(9, "free priv->dibMem\n");
        DeleteObject(priv->dibMem);
        pmesg(9, "free self->priv\n");

        free(self->priv);
        self->priv = 0;
    }

    pmesg(9, "free self\n");
    free(self);
    pmesg(9, "screenshot: end dealloc\n");
}

screenshot_t* screenshot_new()
{
    screenshot_t *ss = (screenshot_t*) calloc(sizeof(screenshot_t), 1);

    ss->priv = (private_t*)calloc(sizeof(private_t), 1);

    //ss->init = screenshot_init;
    //ss->get_image = screenshot_get_image;
    //ss->get_screen_size = screenshot_get_screen_size;
    //ss->free_image = screenshot_free_image;
    //ss->dealloc = screenshot_dealloc;

    return ss;
}

