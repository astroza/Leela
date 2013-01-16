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
	char buf[16];
	int fd;
	/*
	 * bit 5 set : INPUT
	 * bit 4 set : PULL UP
	 * bit 3 clear: PULL ENABLED
	 * bit 2, 1, 0 set: MODE 7 para GPIO1_17
	*/
	unsigned char conf = 32 + 16 + 0 + 4 + 2 + 1;
	snprintf(buf, sizeof(buf), "%x", conf);
	fd = open("/sys/kernel/debug/omap_mux/gpmc_a1", O_WRONLY);
	if(fd == -1) {
		perror("open");
		return -1;
	}
	write(fd, buf, strlen(buf));
	close(fd);

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd == -1) {
		perror("open");
		return -1;
	}
	/* GPIO1_17
	 * 1 * 32 + 17 = 49
	 */
	write(fd, "49", 2);
	close(fd);

	fd = open("/sys/class/gpio/gpio49/direction", O_WRONLY);
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

	out->value_fd = open("/sys/class/gpio/gpio49/value", O_RDONLY);
	if(out->value_fd == -1) {
		if(!setup_was_run) {
			setup_gpio();
			setup_was_run = 1;
			return open_button(out);
		} 
		perror("open");
		return -1;
	}

	out->edge_fd = open("/sys/class/gpio/gpio49/edge", O_WRONLY);
	return 0;
}

int wait_for_rising(button *b)
{
	char value;
	fd_set rset;

	FD_ZERO(&rset);
	FD_SET(b->value_fd, &rset);
	write(b->edge_fd, "rising", 6);
	select(b->value_fd + 1, NULL, NULL, &rset, NULL);
	return read(b->value_fd, &value, 1);
}

int wait_for_falling(button *b)
{
        char value;
        fd_set rset;

        FD_ZERO(&rset);
        FD_SET(b->value_fd, &rset);
        write(b->edge_fd, "falling", 7);
        select(b->value_fd + 1, NULL, NULL, &rset, NULL);
        return read(b->value_fd, &value, 1);
}

int stop_process(int pid)
{
	return kill(pid, 15);
}

int start_process(char *path, char *arg)
{
	char *const argv[] = {path, arg, NULL};
	int pid;
	int null_fd;

	pid = fork();
	if(pid == 0) {
		null_fd = open("/dev/null", O_WRONLY);
		chdir("/home/root/devel/eye/santiago/");
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
	wait_for_rising(&b);
	do {
		wait_for_rising(&b);
		pid = start_process("/home/root/devel/eye/santiago/contest", "0");
		wait_for_falling(&b);
		stop_process(pid);
		waitpid(pid, &status, 0);
	} while(1);
	return 0;
}
