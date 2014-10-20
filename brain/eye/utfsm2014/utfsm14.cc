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
#include <stdlib.h> 
#include <math.h>

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

	//cout << "Comparing " << v << " with " << old_v << endl;
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
        float s0_distance = wall_sensor0_distance();
	float s1_distance = wall_sensor1_distance();
	float s_diff;
	int dir;

	s_diff = s0_distance - s1_distance;

       if(s_diff < 0)     
		dir = 1;
        else
            	dir = -1;

        cout << "S_DIFF: " << s_diff << endl;
        AUTOM_BEGIN

/*
        WAIT_UNTIL(navig->rotate(-10) && is_increasing(s0_distance));
        navig->stop();
        WAIT_UNTIL(navig->rotate(10) && is_increasing(s0_distance));
        navig->stop();
	// rocket science
*/
	WAIT_UNTIL(navig->rotate(10*dir) && fabs(s_diff) <= 1.0);
	navig->stop();
	
/*
	if(s0_distance > 40) {
        	timer.start(500);
        	WAIT_WHILE(navig->rotate(-10) && timer.is_running());
        	navig->stop();
	}
*/
	return -1;                 
        AUTOM_END
	return 0;
}

float wall_sensor0_distance()
{
	float s0_distance = depth_sensor_v2cm(v3_analog_read(0));
	return s0_distance;
}

float wall_sensor1_distance()
{
	float s1_distance = depth_sensor_v2cm(v3_analog_read(1));
	if(wall_sensor0_distance() >= 24)
		s1_distance *= cos(0.460579);
	return s1_distance;
}

#define NEAR_VALUE 600
bool obstacle_in_right_side()
{
	int s2 = v3_analog_read(2);
	return s2 > NEAR_VALUE;
}

bool obstacle_in_left_side()
{
	int s3 = v3_analog_read(3);
	return s3 > NEAR_VALUE;
}

bool obstacle_in_front()
{
	obstacle_in_right_side() || obstacle_in_left_side();
}

bool forward_with_feedback(Navig *navig, int velocity, int cur_distance, int distance_to_keep)
{
	/* 0.1: derecha
         * 5: izquierda
         */
	float rudder = 1.0;

	int diff = distance_to_keep - cur_distance;
	if(diff <= -2)
		rudder = 0.1;
	if(diff >= 2)
		rudder = 5.0;

	return navig->forward(velocity, rudder);
}
