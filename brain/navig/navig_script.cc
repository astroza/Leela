#include <iostream>
#include <unistd.h>
#include <fstream>
using namespace std;
#include "navig_script.h"
#include <automata.h>

bool NavigScript::open(const char *filename)
{
	if(script.is_open())
		script.close();

	script.open(filename);
	return !script.fail();
}

bool NavigScript::read_line()
{
	if(script.eof())
		return false;

	val1 = 0;

	script >> cmd;
	switch(cmd) {
        	case 'f':
                	script >> val0;
                        script >> val2;
                        script >> val1;
                        break;
		case 'r':
                        script >> val0;
                        script >> val1;
                        break;
                case 'c':
                        script >> val0;
                        script >> val1;
                        break;
                case 'a':
                        script >> val0;
                        script >> val1;
                        break;
                case 'p':
                        script >> val0;
                        script >> val1;
                        break;
                case 'b':
                        script >> val0;
                        script >> val1;
                        break;
                default:
                case 's':
			break;
	}

	return true;
}

void NavigScript::exec_line()
{
        switch(cmd) {
                case 'f':
                        navig->forward((short)val0, val2);
                        break;
                case 'r':
                        navig->rotate((short)val0);
                        break;
                case 'c':
                        navig->crab_move((short)val0);
                        break;
                case 'a':
                        picker->close_arms(val0);
                        break;
                case 'p':
                        picker->rotate_arms(val0);
                        break;
                case 'b':
                        picker->rotate_base(val0 == 0? false : true);
                        break;
                default:
                case 's':
                        navig->stop();
	}
}

int NavigScript::do_step()
{
	unsigned long long diff;
	struct timeval now, time_diff;
	switch(current_step) {
		case 0:
			if(read_line() == false) /* EOF */
				return -1;
			exec_line();
			gettimeofday(&line_start, NULL);
			current_step++;
			break;
		case 1:
			exec_line();
			gettimeofday(&now, NULL);
			timersub(&now, &line_start, &time_diff);
			diff = (time_diff.tv_sec * 10) + (time_diff.tv_usec/100000.0);
			if(diff > val1)
				current_step = 0;
	}
	return 0;
}

void NavigScript::init(Navig *navig, Picker *picker)
{
        this->navig = navig;
        this->picker = picker;
}

NavigScript::NavigScript()
{
	init(NULL, NULL);
}

NavigScript::NavigScript(Navig *navig, Picker *picker)
{
	init(navig, picker);
}
