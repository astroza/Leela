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

#include "v3.h"
#include <stdio.h>

int main()
{
	v3_open();
	v3_analog_inputs_enable(V3_A0|V3_A1|V3_A2);

	do {
		printf("value=%d\n", v3_analog_read(0));
		v3_work();
		usleep(500000);
	} while(1);

	return 0;
}
