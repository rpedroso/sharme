#include "colorspace.h"

inline void rgb2yuv420p(const unsigned char *rgb, unsigned char *yuv420p, const int width, const int height, const int depth)
{
  unsigned int j, k, i = 0;
  unsigned int numpixels = width * height;
  unsigned int ui = numpixels;
  unsigned int vi = numpixels + (numpixels >> 2);
  unsigned int s = 0;

#define sR (rgb[s+2])
#define sG (rgb[s+1])
#define sB (rgb[s+0])
//#define sR ((rgb[s+2]>>5)<<5)
//#define sG ((rgb[s+1]>>5)<<5)
//#define sB ((rgb[s+0]>>5)<<5)
//#define sR (rgb[s+2]&0xf7)
//#define sG (rgb[s+1]&0xf7)
//#define sB (rgb[s+0]&0xf7)

  for (j=0; j < height; j++)
  {
    for (k=0; k < width; k++, i++)
    {
      yuv420p[i] = ( (66*sR + 129*sG + 25*sB + 128) >> 8) + 16;
      //yuv420p[i] = (0.257 * sR) + (0.504 * sG) + (0.098 * sB) + 16;
      //yuv420p[i] = (sR + sG) >> 1;

      if ((0 == j%2) && (0 == k%2))
      {
        yuv420p[ui++] = ( (-38*sR - 74*sG + 112*sB + 128) >> 8) + 128;
        //yuv420p[ui++] = -(0.148 * sR) - (0.291 * sG) + (0.439 * sB) + 128;
        yuv420p[vi++] = ( (112*sR - 94*sG - 18*sB + 128) >> 8) + 128;
        //yuv420p[vi++] = (0.439 * sR) - (0.368 * sG) - (0.071 * sB) + 128;
      }
      s += depth;
    }
  }
}



/* Converts a YUV planar frame of width "d->c->width and height "d->c->height" at "src" straight
 * into a RGB24 frame at "dst" (must be allocated y caller).
 * The following is taken from libv4l written by Hans de Goede <j.w.r.degoede@hhs.nl>
 * (See CREDITS)
 */
inline void yuv420p2rgb(const unsigned char *yuv420p, unsigned char *rgb, const int width, const int height, const int depth)
{
    int i,j, w = width, h = height, u1, rg, v1;
    const unsigned char  *y = yuv420p, *u = yuv420p + w * h, *v = u + ((w * h) >> 2);
    
#define CLIP(x) (unsigned char) ((x) > 255) ? 255 : (((x) < 0) ? 0 : (x));
#define R(u) (((*u - 128) << 7) + (*u - 128)) >> 6;
#define G(u,v) (((*u - 128) << 1) +  (*u - 128) + ((*v - 128) << 2) + ((*v - 128) << 1)) >> 3;
#define B(v) (((*v - 128) << 1) +  (*v - 128)) >> 1;

    //dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
    
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j += 2) {
            u1 = R(u); //(((*u - 128) << 7) +  (*u - 128)) >> 6;
            rg = G(u,v); //(((*u - 128) << 1) +  (*u - 128) + ((*v - 128) << 2) + ((*v - 128) << 1)) >> 3;
            v1 = B(v); //(((*v - 128) << 1) +  (*v - 128)) >> 1;
            
            *rgb++ = CLIP(*y + v1);
            *rgb++ = CLIP(*y - rg);
            *rgb++ = CLIP(*y + u1);
            y++;
            
            *rgb++ = CLIP(*y + v1);
            *rgb++ = CLIP(*y - rg);
            *rgb++ = CLIP(*y + u1);
            
            y++;
            u++;
            v++;
        }
        
        if (i&1) {
            u -= w >> 1;
            v -= w >> 1;
        }
    }
}
