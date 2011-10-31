#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _screenshot
{
    void *priv;
    unsigned char *data;
    //void (*init)(struct _screenshot *self, int startx, int starty, int width, int height);
    //void (*get_image)(struct _screenshot *self); //, int startx, int starty, int width, int height);
    //void (*get_screen_size)(struct _screenshot *self, int index, int *width, int *height);
    //void (*free_image)(struct _screenshot *self);
    //void (*dealloc)(struct _screenshot *self);
} screenshot_t;

screenshot_t* screenshot_new();
    void screenshot_init(struct _screenshot *self, int startx, int starty, int width, int height);
    void screenshot_get_image(struct _screenshot *self); //, int startx, int starty, int width, int height);
    void screenshot_get_screen_size(struct _screenshot *self, int index, int *width, int *height);
    int screenshot_get_depth(screenshot_t *self);
    void screenshot_free_image(struct _screenshot *self);
    void screenshot_dealloc(struct _screenshot *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
