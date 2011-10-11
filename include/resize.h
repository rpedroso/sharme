#ifndef RESIZE_H
#define RESIZE_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef uchar
#define uchar unsigned char
#endif

void resample(uchar *src, int src_w, int src_h, int d, uchar *dst, int dst_w, int dst_h);
void resample_nearest(uchar *src, int width, int height, int d, uchar *dst, int dst_w, int dst_h);
void resample_box(uchar *src, int width, int height, int d, uchar *dst, int dst_w, int dst_h);
inline void vs_image_scale_nearest_RGB(uchar *src, int src_w, int src_h, int d, uchar *dst, int dst_w, int dst_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
