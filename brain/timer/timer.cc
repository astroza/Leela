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
