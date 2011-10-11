#include "resize.h"

//#define RGB16(red, green, blue) ( ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))

inline void resample_nearest(uchar *src, int width, int height, int d, uchar *dst, int dst_w, int dst_h)
{
    unsigned long x_delta = (width<<16) / dst_w;
    unsigned long y_delta = (height<<16) / dst_h;

    long y = 0;
    long j = 0;
    //for (j = 0; j < dst_h; j+=2)
    for (j = 0; j < dst_h; j++)
    {
        unsigned char* src_line = &src[(y>>16)*width*d];

        long x = 0;
        long i = 0;
        for (i = 0; i < dst_w; i++)
        {
            unsigned char* src_pixel = &src_line[(x>>16)*d];
            dst[0] = src_pixel[0];
            dst[1] = src_pixel[1];
            dst[2] = src_pixel[2];
            dst += 3;
            x += x_delta;
        }

        //y += y_delta*2;
        y += y_delta;
    }
}

inline void resample_box(uchar *src, int width, int height, int d, uchar *dst, int dst_w, int dst_h)
{
    // This function implements a simple pre-blur/box averaging method for
    // downsampling that gives reasonably smooth results To scale the image
    // down we will need to gather a grid of pixels of the size of the scale
    // factor in each direction and then do an averaging of the pixels.

    const double scale_factor_x = (double) width / dst_w;
    const double scale_factor_y = (double) height / dst_h;

    const int scale_factor_x_2 = (int)(scale_factor_x / 2);
    const int scale_factor_y_2 = (int)(scale_factor_y / 2);

    int averaged_pixels, src_pixel_index;
    double sum_r, sum_g, sum_b, sum_a;

    int y;
    for (y = 0; y < dst_h; y++)         // Destination image - Y direction
    {
        // Source pixel in the Y direction
        int src_y = (int)(y * scale_factor_y);

        int x;
        for (x = 0; x < dst_w; x++)      // Destination image - X direction
        {
            // Source pixel in the X direction
            int src_x = (int)(x * scale_factor_x);

            // Box of pixels to average
            averaged_pixels = 0;
            sum_r = sum_g = sum_b = sum_a = 0.0;

            int j;
            for (j = (int)src_y - scale_factor_y/2.0 + 1; j <= (int)src_y + scale_factor_y_2; j++ )
            {
                // We don't care to average pixels that don't exist (edges)
                if ( j < 0 || j > height - 1 )
                    continue;

                int i;
                for (i = (int)src_x - scale_factor_x/2.0 + 1; i <= src_x + scale_factor_x_2; i++ )
                {
                    // Don't average edge pixels
                    if ( i < 0 || i > width - 1 )
                    if ( i < 0 || i > width - 1 )
                        continue;

                    // Calculate the actual index in our source pixels
                    src_pixel_index = j * width + i;

                    sum_r += src[src_pixel_index * d + 0];
                    sum_g += src[src_pixel_index * d + 1];
                    sum_b += src[src_pixel_index * d + 2];

                    averaged_pixels++;
                }
            }

            // Calculate the average from the sum and number of averaged pixels
            dst[0] = (unsigned char)(sum_r / averaged_pixels);
            dst[1] = (unsigned char)(sum_g / averaged_pixels);
            dst[2] = (unsigned char)(sum_b / averaged_pixels);
            dst += 3;
        }
    }
}





static inline void linear(uchar *src, int d, int lo, int hi, double frac, uchar *dst)
{
  int i;

  for(i=0; i<d; i++,lo++,hi++)
  {
    dst[i] = (uchar)(src[lo] + ((src[hi] - src[lo]) * frac) + 0.5);
  }
}

static inline void bilinear(uchar *src, int d, int sw, int se, int nw, int ne, double fx, double fy, uchar *dst)
{
  double s, n;
  int i;

  for(i=0; i<d; i++,sw++,se++,nw++,ne++)
  {
    s = src[sw] + ((src[se] - src[sw]) * fx);
    n = src[nw] + ((src[ne] - src[nw]) * fx);
    dst[i] = (uchar)(s + ((n - s) * fy) + 0.5);
  }
}

inline void resample(uchar *src, int src_w, int src_h, int d, uchar *dst, int dst_w, int dst_h)
{
  double sx, sy, fx, fy, u, v;
  int i, j, k, sw, se, nw, ne, s, t, dx, dy, src_ex, src_ey, dst_ex, dst_ey;

  src_ex = (src_w - 1);
  src_ey = (src_h - 1);

  dst_ex = (dst_w - 1);
  dst_ey = (dst_h - 1);

  /* hack: ensures that sx/sy division produces 0 index when w/h are 1 pix
  */
  sx = (src_ex)? dst_ex/(double)src_ex : (double)dst_w;
  sy = (src_ey)? dst_ey/(double)src_ey : (double)dst_h;

  /* bottom row
  */
  k = 0;
  dy = 0; t = 0; fy = 0.0;

  /* left column
  */
  dx = 0; s = 0; fx = 0.0;
  sw = (t*src_w + s) * d; se = sw + dx;
  linear(src, d, sw, se, fx, &dst[k]);
  k+=3;

  /* interior columns
  */
  dx = (src_ex)? d : 0;
  for(i=1; i<dst_ex; i++)
  {
    u = i/sx; s = (int)u; fx = u - s;
    sw = (t*src_w + s) * d; se = sw + dx;
    linear(src, d, sw, se, fx, &dst[k]);
    k+=3;
  }

  /* right column
  */
  if(dst_ex)
  {
    dx = 0; s = src_ex; fx = 0.0;
    sw = (t*src_w + s) * d; se = sw + dx;
    linear(src, d, sw, se, fx, &dst[k]);
    k+=3;
  }

  /* interior rows
  */
  dy = (src_ey)? src_w * d : 0;
  for(j=1; j<dst_ey; j++)
  {
    v = j/sy; t = (int)v; fy = v - t;

    /* left column
    */
    dx = 0; s = 0; fx = 0.0;
    sw = (t*src_w + s) * d; nw = sw + dy;
    linear(src, d, sw, nw, fy, &dst[k]);
    k+=3;

    /* interior columns
    */
    dx = (src_ex)? d : 0;
    for(i=1; i<dst_ex; i++)
    {
      u = i/sx; s = (int)u; fx = u - s;
      sw = (t*src_w + s) * d; se = sw + dx; nw = sw + dy; ne = nw + dx;
      bilinear(src, d, sw, se, nw, ne, fx, fy, &dst[k]);
      k+=3;
    }

    /* right column
    */
    if(dst_ex)
    {
      dx = 0; s = src_ex; fx = 0.0;
      sw = (t*src_w + s) * d; nw = sw + dy;
      linear(src, d, sw, nw, fy, &dst[k]);
      k+=3;
    }
  }

  /* top row
  */
  if(dst_ey)
  {
    dy = 0; t = src_ey; fy = 0.0;

    /* left column
    */
    dx = 0; s = 0; fx = 0.0;
    sw = (t*src_w + s) * d; se = sw + dx;
    linear(src, d, sw, se, fx, &dst[k]);
    k+=3;

    /* interior columns
    */
    dx = (src_ex)? d : 0;
    for(i=1; i<dst_ex; i++)
    {
      u = i/sx; s = (int)u; fx = u - s;
      sw = (t*src_w + s) * d; se = sw + dx;
      linear(src, d, sw, se, fx, &dst[k]);
      k+=3;
    }

    /* right column
    */
    if(dst_ex)
    {
      dx = 0; s = src_ex; fx = 0.0;
      sw = (t*src_w + s) * d; se = sw + dx;
      linear(src, d, sw, se, fx, &dst[k]);
      k+=3;
    }
  }
}





//void
//vs_image_scale_nearest_RGB (const VSImage * dest, const VSImage * src,
//    uint8_t * tmpbuf)
inline void
vs_scanline_resample_nearest_RGB (uchar * dest, uchar * src, int src_width,
    int n, int *accumulator, int increment)
{
  int acc = *accumulator;
  int i;
  int j;
  int x;

  for (i = 0; i < n; i++) {
    j = acc >> 16;
    x = acc & 0xffff;
    dest[i * 4 + 0] = (x < 32768
        || j + 1 >= src_width) ? src[j * 4 + 0] : src[j * 4 + 3];
    dest[i * 4 + 1] = (x < 32768 
        || j + 1 >= src_width) ? src[j * 4 + 1] : src[j * 4 + 4];
    dest[i * 4 + 2] = (x < 32768
        || j + 1 >= src_width) ? src[j * 4 + 2] : src[j * 4 + 5];
  
    acc += increment;
  }
  
  *accumulator = acc;
} 

inline void vs_image_scale_nearest_RGB(uchar *src, int src_w, int src_h, int d, uchar *dst, int dst_w, int dst_h)

{ 
  int acc; 
  int y_increment;
  int x_increment;
  int i;
  int j;
  int xacc;

  if (dst_h == 1)
    y_increment = 0;
  else
    y_increment = ((src_h - 1) << 16) / (dst_h - 1);

  if (dst_w == 1)
    x_increment = 0;
  else
    x_increment = ((src_w - 1) << 16) / (dst_w - 1);
    
  acc = 0; 
  for (i = 0; i < dst_h; i++) {
    j = acc >> 16;
    
    xacc = 0;
    vs_scanline_resample_nearest_RGB (dst + i * d,
        src + j * d, src_w, dst_w, &xacc,
        x_increment);
  
    acc += y_increment;
  }
} 

