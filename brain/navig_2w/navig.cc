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

#include <iostream>
#include "navig.h"
#include <math.h>

using namespace std;

extern "C" {
#include <v4.h>
#include <mouse.h>
}

SContAdjust::SContAdjust(short forward_adjust, short backward_adjust, bool reverse)
{
        adjust[0] = backward_adjust;
        adjust[1] = forward_adjust;
        this->reverse = reverse;
}

short SContAdjust::calc_PWM(short value)
{
        short ret = value % 2001;

        if(reverse)
                ret = 3000 - value;
#if DEBUG
	cout << "SContAdjust::calc_PWM(" << value << "): after reverse ret = " << ret << endl;
#endif
        if(ret > 1500)
                ret += adjust[1];
        else if(ret < 1500)
                ret += adjust[0];
#if DEBUG
        cout <<	"SContAdjust::calc_PWM(" << value << "): after adjust ret = " << ret << endl;
#endif
        return ret;
}

SContAdjust::SContAdjust()
{
}

int Wheel::raw_velocity_set(short value)
{
	return v4_servo_set(velocity_servo_id, value);
}

int Wheel::velocity_set(short vel)
{
	if(power_off_in_rest && vel == 0)
		return v4_servo_stop(velocity_servo_id);

	short pvalue = 1500 + (vel%101)/100.0*500;
#if DEBUG
	cout << "Wheel::velocity_set[servo_id=" << velocity_servo_id << "](" << vel << ")" << endl;
#endif
	return raw_velocity_set(velocity_adjust.calc_PWM(pvalue));
}

int Wheel::power_off()
{
	return v4_servo_stop(velocity_servo_id);
}

Wheel::Wheel(short vel_id, const SContAdjust &velocity_adjust, bool power_off_in_rest)
{
	velocity_servo_id = vel_id;
	this->velocity_adjust = velocity_adjust;
	this->power_off_in_rest = power_off_in_rest;
}

static Wheel wheels[2] = {Wheel(0, SContAdjust(0, 0, true)), 
			  Wheel(1, SContAdjust(0, 0, false))};

bool Navig::rotate(short velocity)
{
	wheels[0].velocity_set(velocity);
	wheels[1].velocity_set(-velocity);
	return true;
}

bool Navig::rotate_with_angle(short angle)
{
	double advanced;
	if(action != ROTATE) {
		rotate_start_pos = mouse_x_pos;
		rotate_distance = angle*(21/90.0);
		rotate_sign = angle/fabs(angle);
		action = ROTATE;
	}
	advanced = mouse_x_pos - rotate_start_pos;
#ifdef DEBUG
	cout << "ROTATE ADVANCED: " << advanced << endl;
#endif
	if(advanced < rotate_distance/2.0) {
		rotate(rotate_sign*60);
		return true;
	} else if(advanced < rotate_distance) {
		rotate(rotate_sign*10);
		return true;
	} else {
		stop();
		action = IDLE;
	}
	return false;
}

bool Navig::forward(short velocity, float rudder)
{
	float d = 0.5-rudder;

	wheels[0].velocity_set((0.5+d)*velocity);
	wheels[1].velocity_set((0.5-d)*velocity);
	return true;
}

bool Navig::stop()
{
	wheels[0].velocity_set(0);
        wheels[1].velocity_set(0);
	return true;
}

Navig::Navig()
{
	action = IDLE;
	v4_open();
	mouse_open();
}
