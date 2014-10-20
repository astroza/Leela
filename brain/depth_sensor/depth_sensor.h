#ifndef DEPTH_SENSOR
#define DEPTH_SENSOR

int depth_sensor_open();
unsigned int depth_sensor_read();
float depth_sensor_v2cm(int);

#define DS_SAFE_VALUE 500
#define DS_BORDER_VALUE 280

#define DS_ROBOT_IS_SAFE(value) (value >= DS_SAFE_VALUE)

#endif
