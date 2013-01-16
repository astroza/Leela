#ifndef V3_H
#define V3_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>

typedef struct {
	char cmd;
	char motor;
	short arg;
} v3_msg;

typedef struct {
	short current_value;
	pthread_mutex_t lock;
	unsigned char has_next_msg:1;
	v3_msg next_msg;
} v3_servo;

typedef enum { RIGHT=0, LEFT } v3_side;

int v3_open();
int v3_servo_set(int servo_id, short value);
int v3_servo_disconnect(int servo_id);
void v3_work();
const char *v3_device_name_get();
void v3_close();

#endif
