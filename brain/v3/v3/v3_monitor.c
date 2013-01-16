#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include "v3.h"

/* v3_monitor detiene los servos cuando un programa cierra v3_tty (puerto serie conectado a v3).
 * Su objetivo es evitar que el vehiculo siga andando sin "cerebro"
 */
int main()
{
	struct inotify_event iev;
	int v3_fd, inotify_fd, i;
	v3_msg msg;

	msg.cmd = 'D';
	inotify_fd = inotify_init();
	if(inotify_fd == -1) {
		perror("inotify_init");
		return EXIT_FAILURE;
	}

	if(inotify_add_watch(inotify_fd, v3_device_name_get(), IN_CLOSE_WRITE) == -1) {
		perror("inotify_add_watch");
		return EXIT_FAILURE;
	}

	v3_fd = open(v3_device_name_get(), O_RDWR);

	do {
		if(read(inotify_fd, &iev, sizeof(struct inotify_event)) <= 0) {
			perror("read");
			break; /* Sin duda esto no sucedera */
		}
		usleep(500000);
		ioctl(v3_fd, TCFLSH, 0); /* Ignoro las respuestas en la input queue */
		for(i = 0; i < 12; i++) {
			msg.motor = i;
			write(v3_fd, &msg, sizeof(msg));
		}
	} while(1);

	return EXIT_FAILURE;
}
