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

void NavigScript::init(Navig *navig)
{
        this->navig = navig;
}

NavigScript::NavigScript()
{
	init(NULL);
}

NavigScript::NavigScript(Navig *navig)
{
	init(navig);
}
