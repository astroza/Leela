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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int setup_gpio()
{
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd == -1) {
		perror("open");
		return -1;
	}
	/* 
         * http://www.phys-x.org/rbots/index.php?option=com_content&view=article&id=108:lesson-4-beaglebone-black-button-input&catid=46:beaglebone-black&Itemid=81
	 * GPIO 44 which is pin 12 on connector P8
	 */
	write(fd, "44", 2);
	close(fd);
	
	fd = open("/sys/class/gpio/gpio44/direction", O_WRONLY);
	if(fd == -1) {
		perror("open");
		return -1;
	}
	write(fd, "in", 2);
	close(fd);

	return 0;
}

typedef struct {
	int value_fd;
	int edge_fd;
} button;

int open_button(button *out)
{
	static int setup_was_run = 0;

	out->value_fd = open("/sys/class/gpio/gpio44/value", O_RDONLY);
	if(out->value_fd == -1) {
		if(!setup_was_run) {
			setup_gpio();
			setup_was_run = 1;
			return open_button(out);
		} 
		perror("open");
		return -1;
	}

	out->edge_fd = open("/sys/class/gpio/gpio44/edge", O_WRONLY);
	return 0;
}

int wait_for_falling(button *b)
{
        char value[2];
        fd_set rset;
	int ret;
	struct timeval tv;

	tv.tv_sec = 1;
	tv.tv_usec = 0;

        FD_ZERO(&rset);
	write(b->edge_fd, "falling", 7);

	FD_SET(b->value_fd, &rset);
        select(b->value_fd + 1, NULL, NULL, &rset, NULL);
        ret = read(b->value_fd, value, 2);

	/* Posible evento fantasma */
	FD_SET(b->value_fd, &rset);
	if(select(b->value_fd + 1, NULL, NULL, &rset, &tv) > 0)
		ret = read(b->value_fd, value, 2);

	return ret;
}

int stop_process(int pid)
{
	//return kill(pid, 9);
	system("killall -9 stage1");
	system("killall -9 stage2");
	system("killall -9 stage3");
	system("killall -9 stage4");
	return 0;
}

int start_process(char *path, char *arg)
{
	char *const argv[] = {path, arg, NULL};
	int pid;
	int null_fd;

	pid = fork();
	if(pid == 0) {
		null_fd = open("/dev/null", O_WRONLY);
		chdir("/home/root/");
		dup2(null_fd, 1);
		if(execv(path, argv) == -1) {
			perror("execv");
			exit(1);
		 }
	}
	return pid;
}

int main()
{
	button b;
	int pid;
	int status;

	open_button(&b);
	wait_for_falling(&b);
	do {
		puts("WAITING START");
		wait_for_falling(&b);
		pid = start_process("/home/root/stage", "0");
		puts("WAITING STOP");
		wait_for_falling(&b);
		stop_process(pid);
		waitpid(pid, &status, 0);
	} while(1);
	return 0;
}
