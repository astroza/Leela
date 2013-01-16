#include <iostream>
#include <stdlib.h>
#include "timer.h"
#include <sys/time.h>

void Timer::start(unsigned long time)
{
	gettimeofday(&start_time, NULL);
	this->time = time;
}

bool Timer::is_running()
{
	struct timeval now, elapsed_time;
	unsigned long diff;

	gettimeofday(&now, NULL);
	timersub(&now, &start_time, &elapsed_time);
	diff = elapsed_time.tv_sec*1000 + elapsed_time.tv_usec/1000.0;
#ifdef DEBUG
	std::cout << "Timer elapsed time: " << diff << std::endl;
#endif
	if(diff >= time)
		return false;

	return true;
}

Timer::Timer()
{
	this->time = 0;
}
