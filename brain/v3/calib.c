#include "v3.h"
#include <stdio.h>


int set_servos(int fd, short vel)
{
	short i;
	for(i = 0; i < 4; i++) {
		v3_servo_set(fd, i, vel);
	}
}

int disconnect_servos(int fd)
{
	short i;
	for(i = 0; i < 4; i++) {
		v3_servo_disconnect(fd, i);
	}
}

int main()
{
	int fd = v3_open();
	short vels_dir1[] = {100, 110, 120, 130, 140, 150, 160, 170, 180};
	short vels_dir2[] = {80, 70, 60, 50, 40, 30, 20, 10, 0};
	short val0, val1;
	int ret, i;
	char cmd;

	if(fd == -1) {
		puts("Can't open v3 controller");
		return -1;
	}

	do {
		puts("Pot calibration for continuous servos.");
		puts("Setting to 90 (rest position)");
		set_servos(fd, 90);
		puts("Please adjust servos if necessary\n\tPress enter to continue");
		getc(stdin);
		puts("DIR 1");
		for(i = 0; i < 9; i++) {
			printf("Setting servos to (vel=%d)\n", vels_dir1[i]);
			set_servos(fd, vels_dir1[i]);
			puts("Please adjust servos if necessary\n\tPress enter to continue");
			getc(stdin);
		}
		puts("DIR 2");
                for(i =	0; i < 9; i++) {
                        printf("Setting servos to (vel=%d)\n", vels_dir2[i]);
                        set_servos(fd, vels_dir2[i]);
                        puts("Please adjust servos if necessary\n\tPress enter to continue");
                        getc(stdin);
                }

		puts("Repeat all again?");
	} while(getc(stdin) == 'y');
	disconnect_servos(fd);

	return 0;
}
