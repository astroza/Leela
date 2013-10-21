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

#ifndef NAVIG_H
#define NAVIG_H
#include <automata.h>
#include <timer.h>

class SContAdjust
{
	private:
		short adjust[2];
		bool reverse;
	public:
		SContAdjust(short forward_adjust, short backward_adjust, bool reverse=false);
		SContAdjust();
		short calc_PWM(short value);
};

class Wheel {
	private:
		short velocity_servo_id;
		short angle_servo_id;
		SContAdjust velocity_adjust;
		bool power_off_in_rest;
	public:
		short straight_angle_value();
		int raw_velocity_set(short vel);
		int velocity_set(short vel);
		int power_off();
		Wheel(short vel_id, const SContAdjust &velocity_adjust, bool power_off_in_rest=true);
};

#define IDLE	0
#define FORWARD 1
#define ROTATE 	2

class Navig {
	private:
		int action;
		short rotate_sign;
		double rotate_start_pos;
		double rotate_distance;
	public:
		bool rotate_with_angle(short angle);
		bool rotate(short velocity);
		/* velocity de -100 a 100 y radious de 0 a 100 */
		bool forward(short velocity, float rudder);
		bool stop();
		Navig();
};


#endif
