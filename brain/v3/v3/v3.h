/* (c) 2012, 2013 - Felipe Astroza Araya
 *
 *  This file is part of Leela.
 *
 *  Leela is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Leela is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Leela. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef V3_H
#define V3_H

#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define V3_A0 (1 << 0)
#define	V3_A1 (1 << 1)
#define V3_A2 (1 << 2) 
#define V3_A3 (1 << 3)
#define	V3_A4 (1 << 4)
#define	V3_A5 (1 << 5)
#define	V3_A6 (1 << 6)
#define	V3_A7 (1 << 7)

typedef struct {
	char cmd;
	char sel;
	short arg;
} v3_msg;

typedef struct __attribute__ ((__packed__)) {
	char status;
	unsigned short data;
} v3_response;

typedef struct {
	short current_value;
	pthread_mutex_t lock;
	unsigned char has_next_msg:1;
	v3_msg next_msg;
} v3_servo;

typedef enum { RIGHT=0, LEFT } v3_side;

int v3_open();
void v3_analog_inputs_enable(unsigned short bits);
int v3_analog_read(int input);
void v3_launch_init_script();
int v3_servo_set(int servo_id, short value);
int v3_servo_disconnect(int servo_id);
void v3_work();
const char *v3_device_name_get();
void v3_close();

#endif
