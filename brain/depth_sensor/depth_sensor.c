/* (c) 2013 - Felipe Astroza Araya
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

static int sensor_fd = -1;
static const char analog_device[] = "/sys/devices/ocp.2/helper.13/AIN5";

int depth_sensor_open()
{
	int slots_fd;
	sensor_fd = open(analog_device, O_RDONLY);

	if(sensor_fd == -1) {
		slots_fd = open("/sys/devices/bone_capemgr.8/slots", O_WRONLY); 
		write(slots_fd, "cape-bone-iio", 13);
		close(slots_fd);
		sensor_fd = open(analog_device, O_RDONLY);
		if(sensor_fd == -1) {
			fputs("Can't open analog sensor", stderr);
			exit(-1);
		}
	}
	return 0;
}

#define SENSOR_SAMPLES 	10

unsigned int depth_sensor_read()
{
	char buffer[64];
	unsigned int samples[SENSOR_SAMPLES];
	int ret, i;
	float mean = 0, v = 0;
	float sd;
	unsigned int samples_sum = 0;
	unsigned int samples_count = SENSOR_SAMPLES;

	for(i = 0; i < SENSOR_SAMPLES; i++) {
		ret = read(sensor_fd, buffer, sizeof(buffer));
		if(ret == -1) {
			fputs("Error while trying to read analog sensor", stderr);
			samples[i] = 0;
		} else
			samples[i] = strtoul(buffer, NULL, 10);
#ifdef DEBUG
		printf("samples[%d]=%u\n", i, samples[i]);
#endif
		samples_sum += samples[i];
		lseek(sensor_fd, 0, SEEK_SET);
	}
	mean = samples_sum/((float)samples_count);

	/* Varianza */
	for(i = 0; i < SENSOR_SAMPLES; i++)
		v += powf(samples[i] - mean, 2.0);
	v = v/((float)SENSOR_SAMPLES);

	/* Desviacion estandar */
	sd = sqrtf(v);

#ifdef DEBUG
	printf("SD=%f\n", sd);
	printf("PRE SD samples_sum=%u\n", samples_sum);
#endif
	for(i = 0; i < SENSOR_SAMPLES; i++)
		if(samples[i] < mean-sd || samples[i] > mean+sd) {
			samples_sum -= samples[i];
			samples_count--;
		}
#ifdef DEBUG
	printf("POST SD samples_sum=%u\n", samples_sum);
#endif
	return samples_sum/samples_count;
}

int depth_sensor_v2cm(int sensor_value)
{
        float volts = sensor_value*0.00322265625;
        float distance = 65*powf(volts, -1.10);
        return (int)distance;
}

#ifdef UNIT_TEST
int main()
{
	depth_sensor_open();
	while(1) {
		printf("value=%u\n", depth_sensor_read());
		sleep(1);
	}
	return 0;
}
#endif
