#include "v3.h"
#include <stdio.h>

int main()
{
	v3_open();
	short val0, val1;
	int ret;
	char cmd;

	do {
		write(1, "v3> ", 6);
		scanf("%c %hd", &cmd, &val0);
		if(cmd == 'd')
			v3_servo_disconnect(val0);
		else if(cmd == 's') {
			scanf("%hd", &val1);
			v3_servo_set(val0, val1);
		}
		v3_work();
	} while(1);

	return 0;
}
