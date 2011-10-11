#ifndef MOUSE_H
#define MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void mouse_init(void);
void mouse_left_down(void);
void mouse_left_up(void);
void mouse_right_down(void);
void mouse_right_up(void);
void mouse_move(int x, int y);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

