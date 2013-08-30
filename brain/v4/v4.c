#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "v4.h"

#define PWM_PERIOD 20000000

/* Un extremo es 1000uS
 * El centro es 15000uS
 * El otro extremo es 2000uS
 *
 */
 
static v4_struct v4;

static void launch_init_script()
{
	int pid, status;
	char *script_path = "/usr/lib/v4/init.sh";
	char period_string[12];
	char *const args[] = {script_path, period_string, NULL};

	snprintf(period_string, sizeof(period_string), "%d", PWM_PERIOD);
	pid = fork();
	if(pid == 0) {
		exit(execv(script_path, args));
	} else {
		waitpid(pid, &status, 0);
		if(WEXITSTATUS(status) != 0) {
			puts("PWM: Init script execution has an error");
			exit(1);
		}
	}
}

static int _open(const char *path, int flags)
{
	int fd;
	fd = open(path, flags);
	if(fd == -1) {
		perror("open");
		exit(1);
	}
	return fd;
}

void v4_open()
{
	launch_init_script();

	v4.left.run_fd = _open("/tmp/v4/left/run", O_RDWR);
	v4.left.run_state = 0;
	v4.left.duty_fd = _open("/tmp/v4/left/duty", O_RDWR);

	v4.right.run_fd = _open("/tmp/v4/right/run", O_RDWR);
	v4.left.run_state = 0;
	v4.right.duty_fd = _open("/tmp/v4/right/duty", O_RDWR);

	v4.watch_fd = _open("/watch", O_RDONLY);
}

int v4_servo_set(unsigned char servo, short duty_ms)
{
	int duty_ret;
	char value[16];
	int v = duty_ms;

	snprintf(value, sizeof(value), "%d", v*1000);
#ifdef DEBUG
	printf("servo=%d value_ns=%s\n", servo, value);
#endif
	duty_ret = write(v4.pwm[servo].duty_fd, value, strlen(value));
	if(duty_ret != -1 && v4.pwm[servo].run_state == 0)
		v4_servo_start(servo);
	return duty_ret;
}

int v4_servo_start(unsigned char servo)
{
	int ret;

	ret = write(v4.pwm[servo].run_fd, "1", 1);
	if(ret != -1)
		v4.pwm[servo].run_state = 1;
	return ret;
}

int v4_servo_stop(unsigned char servo)
{
	int ret;

	ret = write(v4.pwm[servo].run_fd, "0", 1);
	if(ret != -1)
		v4.pwm[servo].run_state	= 0;
	return ret;
}
