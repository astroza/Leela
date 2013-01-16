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

class S180Adjust
{
	private:
		float factor[2];
		float n2;
		short s_0;
		float calc_lineal_PWM(short angle);
	public:
		S180Adjust(short s_0, short s_45, short s_90, short s_180);
		S180Adjust();
		short calc_PWM(short angle);
};

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

class Picker {
	private:
		int left_arm_id;
		int right_arm_id;
		int picker_id;
		int base_id;
	public:
		Picker(int left_arm=10, int right_arm=11, int picker=9, int base=8);
		int close_arms(short gap);
		int rotate_base(bool for_loading);
		int rotate_arms(short position);
};

class Wheel {
	private:
		short velocity_servo_id;
		short angle_servo_id;
		SContAdjust velocity_adjust;
		S180Adjust angle_adjust;
		bool power_off_in_rest;
	public:
		short straight_angle_value();
		int raw_velocity_set(short vel);
		int raw_angle_set(short vel);
		int velocity_set(short vel);
		int angle_set(short angle);
		int power_off();
		Wheel(short vel_id, short angle_id, const SContAdjust &velocity_adjust, const S180Adjust &angle_adjust, bool power_off_in_rest=true);
};

#define FORWARD 0
#define ROTATE 1
#define CRAB_MOVE 2

class Navig {
	private:
		int action;
		Timer action_timer;
	public:
		static float to_rudder_value(float num, float denom);
		bool rotate(short velocity);
		bool ready_to_rotate();
		/* velocity de -100 a 100 y radious de 0 a 100 */
		bool forward(short velocity, float rudder);
		/* crab move */
		bool crab_move(short velocity);
		bool stop();
		Navig();
};


#endif
