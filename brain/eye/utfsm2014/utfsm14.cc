/* (c) 2013, 2014 - Felipe Astroza Araya
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
#include <timer.h>

extern "C" {
#include <v3.h>
#include <depth_sensor.h>
}

#include "utfsm14.h"

using namespace std;

Aligner::Aligner(Navig *navig)
{
	this->old_v = -1;
	this->navig = navig;
}

bool Aligner::is_increasing(int v)
{
        if(old_v == -1)
                old_v = v;

	cout << "Comparing " << v << " with " << old_v << endl;
        if(v >= old_v+2) {
                old_v = -1;
                return true;
        }
        if(v <= old_v-2) {
		old_v = v;
	}

        return false;
}

int Aligner::do_step()
{
        static Timer timer;
        int s0_distance = depth_sensor_v2cm(v3_analog_read(0));
	cout << "distance = " << s0_distance << endl;
        AUTOM_BEGIN      

        WAIT_UNTIL(navig->rotate(-10) && is_increasing(s0_distance));
        navig->stop();
	cout << "TERMINO GIRO DERECHA" << endl;
        WAIT_UNTIL(navig->rotate(10) && is_increasing(s0_distance));
        navig->stop();
	cout <<	"TERMINO GIRO IZQUIERDA" << endl;
        timer.start(500);
        WAIT_WHILE(navig->rotate(-10) && timer.is_running());
        navig->stop();
	cout << "TERMINO CORRECCION" << endl;

	return -1;                 
        AUTOM_END
	return 0;
}
