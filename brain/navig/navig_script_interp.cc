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
