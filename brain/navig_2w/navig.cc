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

/* Algunas veces es muy violento al comenzar el giro y la cola del robot queda en el aire, eso impide
 * que el mouse pueda leer el suelo
 */
bool Navig::rotate_with_angle(short angle)
{
	double advanced;
	if(action != ROTATE) {
		ra_start_pos = mouse_x_pos;
		ra_distance = fabs(angle)*(21/90.0);
		ra_sign = angle/fabs(angle);
		action = ROTATE;
		rotate(ra_sign*10);
	}
	advanced = fabs(mouse_x_pos - ra_start_pos);
#ifdef DEBUG
	cout << "[rotate_distance]=" << rotate_distance << " [ROTATE ADVANCED]=" << advanced << endl;
#endif
	if(advanced < ra_distance*0.3) {
			rotate(ra_sign*10);
		return true;
	} else if(advanced < ra_distance) {
			rotate(ra_sign*10);
		return true;
	} else {
		stop();
		action = IDLE;
	}
	return false;
}

bool Navig::advance(double distance)
{
	double advanced;

	if(action != ADVANCE) {
		ra_start_pos = mouse_y_pos;
		ra_distance = fabs(distance);
		ra_sign = ra_distance/distance;
		action = ADVANCE;
	}
	advanced = fabs(mouse_y_pos - ra_start_pos);
        cout << "[distance]=" << ra_distance << " [ADVANCED]=" << advanced << endl;
	if(advanced < ra_distance*0.6 && ra_distance-advanced > 3 && ra_sign > 0) {
		forward(ra_sign*60, 0.5);
		return true;
	} else if(advanced < ra_distance) {
		forward(ra_sign*30, 0.5);
		return true;
	} else {
		stop();
		action = IDLE;
	}
	return false;
}

float Navig::to_rudder_value(float num, float denom)
{              
 	float max_value = num > denom ? num : denom;

        return (100.0*(num/max_value)) / (100.0*(denom/max_value));
}

bool Navig::forward(short velocity, float rudder)
{
	float d = 0.5-rudder;

#ifdef DEBUG
	cout << "wheels[0].velocity_set(" << (0.5+d)*velocity << ")" << endl;
	cout << "wheels[1].velocity_set(" << (0.5-d)*velocity << ")" << endl;
#endif

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
