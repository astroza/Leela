#include "navig_script.h"

extern "C" {
#include <v3.h>
}

int main()
{
	AutomataControl control;
	Navig navig;
	Picker picker;
	NavigScript ns(&navig, &picker);

	ns.open("/dev/stdin");
	control.jump(&ns);

	while(control.do_step() != -1) {
		usleep(100000);
		v3_work();
	}

	return 0;
}
