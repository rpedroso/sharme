#ifndef COLORSPACE_H
#define COLORSPACE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


void rgb2yuv420p(const unsigned char *rgb, unsigned char *yuv420p, const int width, const int height, const int depth);
void yuv420p2rgb(const unsigned char *yuv420p, unsigned char *rgb, const int width, const int height, const int depth);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COLORSPACE_H */
