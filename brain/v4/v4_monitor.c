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
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

void stop_servo(const char *run_path)
{
	int fd;

	fd = open(run_path, O_WRONLY);
	perror("open");
	write(fd, "0", 1);
	perror("write");
	close(fd);
}

int main()
{
	struct inotify_event iev;
	int inotify_fd;

	inotify_fd = inotify_init();
	if(inotify_fd == -1) {
		perror("inotify_init");
		return EXIT_FAILURE;
	}

	if(inotify_add_watch(inotify_fd, "/watch", IN_CLOSE_NOWRITE) == -1) {
		perror("inotify_add_watch");
		return EXIT_FAILURE;
	}

	do {
		if(read(inotify_fd, &iev, sizeof(struct inotify_event)) <= 0) {
			perror("read");
			break; /* Sin duda esto no sucedera */
		}
		usleep(500000);
		stop_servo("/tmp/v4/left/run");
		stop_servo("/tmp/v4/right/run");
	} while(1);

	return EXIT_FAILURE;
}
