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
#include <math.h>

float distance(int value)
{
	float volts = value*0.00322265625; 
	float distance = 65*powf(volts, -1.10);
	return distance;
}

/* D1 crecio mas al alejar el robot que D0, significa que sus proyecciones hacia la pared
*  no son paralelas y necesitan una correccion usando trigonometria. A D1 le aplico cos del
* angulo formado entre D0 y D1 para disminuir su diferencia. Si el angulo es 0, cos(0)=1 y D1 se
* mantiene igual, si el angulo es > 0, D1 se achica
*/
int main()
{
	v3_open();
	v3_analog_inputs_enable(V3_A0|V3_A1|V3_A2|V3_A3);
	const float angle = 1.1172;

	do {
		v3_work();
		printf("D0=%f (raw v=%d)\n", distance(v3_analog_read(0)), v3_analog_read(0));
		printf("D1=%f (raw v=%d)\n", (distance(v3_analog_read(1))*cos(0.460579)), v3_analog_read(1));
		printf("D2 (raw v=%d)\n", v3_analog_read(2));
		printf("D3 (raw v=%d)\n", v3_analog_read(3));
		usleep(500000);
	} while(1);

	return 0;
}
