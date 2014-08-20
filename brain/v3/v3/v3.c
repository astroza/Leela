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

#include "v3.h"
#include <sys/uio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/wait.h>

static const char v3_tty[] = "/dev/ttyO1";
static v3_servo v3_servos[12];
static v3_msg *v3_msgs[12];
static v3_response v3_resps[12];
static unsigned int v3_msgs_count = 0;
static int v3_fd = -1;
static unsigned short v3_analog_values[8];
static unsigned short v3_analog_inputs_bits = 0;

static int  __v3_send_msgs(int wait_response)
{
	struct timeval tv;
	fd_set read_fds;
	int read_bytes = 0;
	int bytes_to_read;
	int ret;
	struct iovec msgs_vec[12];
	unsigned int i;

	v3_msg **msgs = v3_msgs;
	unsigned int msgs_count = v3_msgs_count;
	v3_msgs_count = 0;

	for(i = 0; i < msgs_count; i++) {
		msgs_vec[i].iov_base = msgs[i];
		msgs_vec[i].iov_len = sizeof(v3_msg);
	}

	if(writev(v3_fd, msgs_vec, msgs_count) == -1) {
		perror("write");
		return -1;
	}

	if(!wait_response)
		return 0;

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	bytes_to_read = sizeof(v3_response)*msgs_count;
	do {
		FD_ZERO(&read_fds);
                FD_SET(v3_fd, &read_fds);                      
                if(select(v3_fd + 1, &read_fds, NULL, NULL, &tv) < 1)
			return -1;
		ret = read(v3_fd, ((void *)&v3_resps) + read_bytes, bytes_to_read);
		if(ret == -1) {
			perror("read");
			return -1;
		}
		read_bytes += ret;
		bytes_to_read -= ret;
	} while(bytes_to_read > 0);

	return 0;
}

void v3_work()
{
	int i;

	// Servos
	if(v3_msgs_count > 0) {
		for(i = 0; i < v3_msgs_count; i++)
			v3_servos[(int)v3_msgs[i]->sel].has_next_msg = 0;

		__v3_send_msgs(0);

	}

	// Analog inputs
	v3_msg ar_msgs[8];
	v3_msg *msg;
	int ar_count;
	for(i = 0; i < 8; i++) {
		if(v3_analog_inputs_bits & (1 << i)) {
			msg = ar_msgs + i;
			msg->cmd = 'A';
			msg->sel = (char)i;
			msg->arg = 0;
			v3_msgs[v3_msgs_count++] = msg;
		}
	}
	ar_count = v3_msgs_count;
	if(v3_msgs_count > 0 && __v3_send_msgs(1) == 0) {
		for(i = 0; i < ar_count; i++) {
			if(v3_resps[i].status == 'K')
				v3_analog_values[(int)v3_msgs[i]->sel] = v3_resps[i].data;
		}
	}
}

void v3_launch_init_script()
{
        int pid, status;
        char *script_path = "/usr/lib/v3/init.sh";
        char *const args[] = {script_path, NULL};

        pid = fork();
        if(pid == 0) {
		exit(execv(script_path, args));
        } else {
                waitpid(pid, &status, 0);
                if(WEXITSTATUS(status) != 0) {
                        puts("V3: Init script execution has an error");
                        exit(1);
                }
        }
}

void v3_analog_inputs_enable(unsigned short bits)
{
	v3_analog_inputs_bits = bits;
}

int v3_analog_read(int input) 
{
	if(v3_analog_inputs_bits & (1 << input)) {
		return v3_analog_values[input];
	}
	return -1;
}

int v3_open()
{
	struct termios term;
	int fd, i;

	v3_launch_init_script();

	fd = open(v3_tty, O_RDWR);
	if(fd == -1) {
		perror("open");
		return -1;
	}

	if(tcgetattr(fd, &term) == -1) {
		perror("tcgetattr");
		goto error;
	}

	cfmakeraw(&term);
	term.c_cflag |= CREAD;
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 1;
	cfsetispeed(&term, B9600);

	tcflush(fd, TCIFLUSH);
	if(tcsetattr(fd, TCSANOW, &term) == -1) {
		perror("tcsetattr");
		goto error;
	}

	for(i = 0; i < 12; i++) {
		v3_servos[i].current_value = -1;
	}

	v3_fd = fd;
	return 0;
error:
	close(fd);
	return -1;
}

const char *v3_device_name_get()
{
	return v3_tty;
}

int v3_servo_disconnect(int servo_id)
{
        v3_msg *msg = &v3_servos[servo_id].next_msg;
	int old_has_next_msg;

#ifdef DEBUG
        printf("%s(%d).\n", __FUNCTION__, servo_id);
#endif
	v3_servos[servo_id].current_value = -1;
	
      	msg->cmd = 'D';
        msg->sel = servo_id;
	old_has_next_msg = v3_servos[servo_id].has_next_msg;
	v3_servos[servo_id].has_next_msg = 1;

	if(old_has_next_msg == 0)
		v3_msgs[v3_msgs_count++] = msg;

        return 0;
}

int v3_servo_set(int servo_id, short value)
{
        v3_msg *msg = &v3_servos[servo_id].next_msg;
	int old_has_next_msg;

	if(v3_servos[servo_id].current_value == value)
		return 0;
#ifdef DEBUG
        printf("%s(%d, %d).\n", __FUNCTION__, servo_id, value);
#endif
	v3_servos[servo_id].current_value = value;
       	msg->cmd = 'S';
        msg->sel = servo_id;
	msg->arg = value;
	old_has_next_msg = v3_servos[servo_id].has_next_msg;
	v3_servos[servo_id].has_next_msg = 1;
	if(old_has_next_msg == 0)
		v3_msgs[v3_msgs_count++] = msg;
        return 0;
}

void v3_close()
{
#ifdef DEBUG
	printf("%s(%d).\n", __FUNCTION__, v3_fd);
#endif
	close(v3_fd);
}
