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

#include "mouse.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

static int mouse_fd = -1;
volatile double mouse_x_pos = 0;
volatile double mouse_y_pos = 0;

static void *mouse_loop(void *unused);

int mouse_open()
{
	pthread_attr_t attr;
	pthread_t unused;
	int fd;

	fd = open("/dev/input/mice", O_RDONLY);
	mouse_fd = fd;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&unused, &attr, mouse_loop, NULL);

	return 0;
error:
	close(fd);
	return -1;
}

static int mouse_read(mouse_data *data)
{
	char buffer[4];
	int ret;

	ret = read(mouse_fd, buffer, sizeof(buffer));
	if(ret == -1)
			return -1;

	data->x_diff = buffer[1];
	data->y_diff = buffer[2];

	if(buffer[0] & (1 << 7)) {
		puts("Y OVERFLOW");
	}
  	if(buffer[0] & (1 << 6)) {
                puts("X OVERFLOW");
        }
}

static void *mouse_loop(void *unused)
{
	mouse_data data;

	printf("x_diff=%hhd, y_diff=%hhd, X=%f, Y=%f\n", data.x_diff, data.y_diff, mouse_x_pos, mouse_y_pos);
	do {
		mouse_read(&data);
		/* Linux kernel mapeo el valor maximo del delta de 255 a 127 (2:1), para encapsularlo en un char con signo.
		 * 2.54[CM]/1600[CPI]=0.004233, por cada unidad de movimiento del mouse, hay 0.004233[CM]
		 */
		mouse_x_pos += data.x_diff * 0.5 * 0.004233;
		mouse_y_pos += data.y_diff * 0.5 * 0.004233;
#ifdef DEBUG
		printf("x_diff=%hhd, y_diff=%hhd, X=%f, Y=%f\n", data.x_diff, data.y_diff, mouse_x_pos, mouse_y_pos);
#endif
	} while(1);
}

void mouse_close()
{
#ifdef DEBUG
	printf("%s(%d).\n", __FUNCTION__, mouse_fd);
#endif
	close(mouse_fd);
}

#ifdef UNIT_TEST

int main()
{
	mouse_data data;

	mouse_open();
	sleep(5000);
}

#endif
