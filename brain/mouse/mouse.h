#ifndef MOUSE_H
#define MOUSE_H

extern volatile double mouse_x_pos;
extern volatile double mouse_y_pos;

int mouse_open();

typedef struct {
	signed char x_diff;
	signed char y_diff;
} mouse_data;

#endif
