#ifndef TIMER_H
#define TIMER_H
#include <sys/time.h>
class Timer {
	public:
		void start(unsigned long time);
		bool is_running();
		Timer();
	private:
		struct timeval start_time;
		unsigned long time;
};

#endif
