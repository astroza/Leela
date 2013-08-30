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

#include "v4.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
	v4_open();
	short val0, val1;
	char cmd;

	do {
		write(1, "v4> ", 6);
		scanf("%c %hd", &cmd, &val0);
		if(cmd == 'd')
			v4_servo_stop(val0);
		else if(cmd == 's') {
			scanf("%hd", &val1);
			v4_servo_set(val0, val1);
		}
	} while(1);

	return 0;
}
