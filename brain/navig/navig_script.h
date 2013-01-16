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

#ifndef NAVIG_SCRIPT_H
#define NAVIG_SCRIPT_H

#include <sys/time.h>
#include "navig.h"
#include <automata.h>
#include <iostream>
#include <fstream>

class NavigScript : public Automata {
	public:
		int do_step();
		bool open(const	char *filename);
		NavigScript(Navig *navig, Picker *picker);
		NavigScript();
		void init(Navig *navig, Picker *picker);
	private:
		struct timeval line_start;
		int val0, val1;
		float val2;
		char cmd;
		std::ifstream script;
		void exec_line();
		bool read_line();
		Navig *navig;
		Picker *picker;
};

#endif
