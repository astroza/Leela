#ifndef V4_H
#define V4_H

struct sysfs_pwm {
	unsigned char run_state;
	int run_fd;
	int duty_fd;
};

typedef struct {
	union {
		struct {
			struct sysfs_pwm right;
			struct sysfs_pwm left;
		};
		struct sysfs_pwm pwm[2];
	};
	int watch_fd;
} v4_struct;

void v4_open();
int v4_servo_set(unsigned char servo, short velocity);
int v4_servo_start(unsigned char servo);
int v4_servo_stop(unsigned char servo);

#endif
