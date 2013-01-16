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
