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

static const char v3_tty[] = "/dev/ttyO1";
static v3_servo v3_servos[12];
static v3_msg *v3_msgs[12];
static unsigned int v3_msgs_count = 0;
static int v3_fd = -1;

static int  __v3_send_msgs(v3_msg **msgs, unsigned int msgs_count)
{
	struct timeval tv;
	fd_set read_fds;
	char resp[msgs_count];
	unsigned int resp_count = 0;
	int ret;
	struct iovec msgs_vec[12];
	unsigned int i;

	for(i = 0; i < msgs_count; i++) {
		msgs_vec[i].iov_base = msgs[i];
		msgs_vec[i].iov_len = sizeof(v3_msg);
	}

	if(writev(v3_fd, msgs_vec, msgs_count) == -1) {
		perror("write");
		return -1;
	}

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	do {
		FD_ZERO(&read_fds);
                FD_SET(v3_fd, &read_fds);                      
                if(select(v3_fd + 1, &read_fds, NULL, NULL, &tv) < 1)
			return -1;
		ret = read(v3_fd, resp+resp_count, msgs_count);
		if(ret == -1) {
			perror("read");
			return -1;
		}
		resp_count += ret;
	} while(resp_count < msgs_count);

	return 0;
}

void v3_work()
{
	int i;
	if(v3_msgs_count > 0) {
		for(i = 0; i < v3_msgs_count; i++)
			v3_servos[(int)v3_msgs[i]->motor].has_next_msg = 0;

		__v3_send_msgs(v3_msgs, v3_msgs_count);
	}
	v3_msgs_count = 0;
}

int v3_open()
{
	struct termios term;
	int fd, i;

	fd = open("/sys/kernel/debug/omap_mux/uart1_rxd", O_WRONLY);
	write(fd, "20\n", 3);
	close(fd);
	fd = open("/sys/kernel/debug/omap_mux/uart1_txd", O_WRONLY);
	write(fd, "0\n", 2);
	close(fd);

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
        msg->motor = servo_id;
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
        msg->motor = servo_id;
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
