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
#include <v3.h>
}

float round(float r) 
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

S180Adjust::S180Adjust(short s_0, short s_45, short s_90, short s_180)
{
	if(s_0 > s_180)
		n2 = -(s_0 - s_180)/180.0;
	else
		n2 = (s_180 - s_0)/180.0;

	this->s_0 = s_0;
	factor[0] = s_45 - calc_lineal_PWM(45);
	factor[1] = s_90 - calc_lineal_PWM(90);
}

S180Adjust::S180Adjust()
{
}

float S180Adjust::calc_lineal_PWM(short angle)
{
	return s_0 + n2*angle;
}

short S180Adjust::calc_PWM(short angle)
{
	float lineal_value = calc_lineal_PWM(angle);
	return round(lineal_value + factor[0]*powf(M_E, -fabs(45-angle)) + factor[1]*powf(M_E, -fabs(90-angle)));
}

SContAdjust::SContAdjust(short forward_adjust, short backward_adjust, bool reverse)
{
        adjust[0] = backward_adjust;
        adjust[1] = forward_adjust;
        this->reverse = reverse;
}

short SContAdjust::calc_PWM(short value)
{
        short ret = value % 180;

        if(reverse)
                ret = 180 - value;

#if DEBUG
	cout << "SContAdjust::calc_PWM(" << value << "): after reverse ret = " << ret << endl;
#endif
        if(ret > 90)
                ret += adjust[1];
        else if(ret < 90)
                ret += adjust[0];
#if DEBUG
        cout <<	"SContAdjust::calc_PWM(" << value << "): after adjust ret = " << ret << endl;
#endif
        return ret;
}

SContAdjust::SContAdjust()
{
}

Picker::Picker(int left_arm, int right_arm, int picker, int base)
{
	left_arm_id = left_arm;
	right_arm_id = right_arm;
	picker_id = picker;
	base_id = base;
}

int Picker::close_arms(short gap)
{
	short _gap = gap;
	if(_gap > 100)
		_gap = 100;
	if(_gap < 0)
		_gap = 0;

	v3_servo_set(left_arm_id, 80+gap);
	v3_servo_set(right_arm_id, 80+gap);
	return 0;
}

int Picker::rotate_base(bool for_loading)
{
	short angle;
	if(for_loading)
		angle = 180;
	else
		angle = 0;

	return v3_servo_set(base_id, angle);
}

int Picker::rotate_arms(short position)
{
#define RA_MIN 10
#define RA_MAX 170
	short v = (position * ((RA_MAX-RA_MIN)/100.0)) + RA_MIN;
	return v3_servo_set(picker_id, v);
}

short Wheel::straight_angle_value()
{
	return angle_adjust.calc_PWM(0);
}

int Wheel::raw_velocity_set(short value)
{
	return v3_servo_set(velocity_servo_id, value);
}

int Wheel::velocity_set(short vel)
{
	if(power_off_in_rest && vel == 0)
		return v3_servo_disconnect(velocity_servo_id);

	short pvalue = 90 + (vel%101)/100.0*30;
#if DEBUG
	cout << "Wheel::velocity_set[servo_id=" << velocity_servo_id << "](" << vel << ")" << endl;
#endif
	return raw_velocity_set(velocity_adjust.calc_PWM(pvalue));
}

int Wheel::raw_angle_set(short value)
{
	return v3_servo_set(angle_servo_id, value);
}

int Wheel::angle_set(short angle)
{
	return raw_angle_set(angle_adjust.calc_PWM(angle));
}

int Wheel::power_off()
{
	return v3_servo_disconnect(velocity_servo_id);
}

Wheel::Wheel(short vel_id, short angle_id, const SContAdjust &velocity_adjust, const S180Adjust &angle_adjust, bool power_off_in_rest)
{
	velocity_servo_id = vel_id;
	angle_servo_id = angle_id;
	this->velocity_adjust = velocity_adjust;
	this->angle_adjust = angle_adjust;
	this->power_off_in_rest = power_off_in_rest;
}

static Wheel wheels[4] = {Wheel(0, 4, SContAdjust(0, 0, true), S180Adjust(125, 80, 43, 0)), 
			  Wheel(1, 5, SContAdjust(2, 0, true), S180Adjust(38, 70, 112, 180)), 
			  Wheel(2, 6, SContAdjust(0, -3), S180Adjust(30, 65, 105, 180)), 
			  Wheel(3, 7, SContAdjust(0, 0), S180Adjust(132, 85, 50, 0))};

// static Picker picker = Picker(10, 11, 9, 8);

bool Navig::ready_to_rotate()
{
	return !action_timer.is_running();
}

bool Navig::rotate(short velocity)
{
	wheels[0].angle_set(45);
	wheels[1].angle_set(45);
	wheels[2].angle_set(45);
	wheels[3].angle_set(45);

	if(action != ROTATE) {
		action = ROTATE;
		action_timer.start(500);
		wheels[0].velocity_set(0);
		wheels[1].velocity_set(0);
		wheels[2].velocity_set(0);
		wheels[3].velocity_set(0);
	} else if(!action_timer.is_running()) {
		wheels[0].velocity_set(velocity);
		wheels[1].velocity_set(velocity);
		wheels[2].velocity_set(-velocity);
		wheels[3].velocity_set(-velocity);
	}
	return true;
}

float Navig::to_rudder_value(float num, float denom)
{
	float max_value = num > denom ? num : denom;

	return (100.0*(num/max_value)) / (100.0*(denom/max_value));
}

bool Navig::forward(short velocity, float rudder)
{
	float side_factor[2] = {1.0, 1.0};
	short straight_value;

	/* Esto produce un movimiento tipo culebra */

	if(rudder < 5.0 && rudder > 0.2) {
		wheels[0].angle_set(0);
        	wheels[1].angle_set(0);
        	wheels[2].angle_set(0);
        	wheels[3].angle_set(0);
	} else if(rudder > 1.0) { /* right */
		/* 150 */
		straight_value = wheels[0].straight_angle_value();
                wheels[0].raw_angle_set(straight_value - ((straight_value - 150)*(0.5 + 0.5*rudder/100.0)));
		/* 10 */
		straight_value = wheels[1].straight_angle_value();
                wheels[1].raw_angle_set(straight_value - ((straight_value - 10)*(0.5 + 0.5*rudder/100.0)));
		/* 60 */
		straight_value = wheels[2].straight_angle_value();
                wheels[2].raw_angle_set(straight_value - ((straight_value - 60)*(0.5 + 0.5*rudder/100.0)));
		/* 95 */
		straight_value = wheels[3].straight_angle_value();
                wheels[3].raw_angle_set(straight_value - ((straight_value - 95)*(0.5 + 0.5*rudder/100.0)));

		/* El lado izquierdo decae en velocidad hasta un 75% */
		side_factor[0] *= 1-0.25*(rudder/100.0); 
	} else { /* left */
		/* 90 */
		straight_value = wheels[0].straight_angle_value();
                wheels[0].raw_angle_set(straight_value - ((straight_value - 90)*(0.3 + 0.7*0.01/rudder)));
		/* 70 */
		straight_value = wheels[1].straight_angle_value();
                wheels[1].raw_angle_set(straight_value - ((straight_value - 70)*(0.3 + 0.7*0.01/rudder)));
		/* 0 */
		straight_value = wheels[2].straight_angle_value();
                wheels[2].raw_angle_set(straight_value - ((straight_value - 0)*(0.3 + 0.7*0.01/rudder)));
		/* 170 */
		straight_value = wheels[3].straight_angle_value();
                wheels[3].raw_angle_set(straight_value - ((straight_value - 170)*(0.3 + 0.7*0.01/rudder)));

		/* El lado derecho decae en velocidad hasta un 75% */
		side_factor[1] *= 1-0.25*(0.01/rudder);
	}

	if(action != FORWARD) {
		action = FORWARD;
		action_timer.start(500);
                wheels[0].velocity_set(0);
                wheels[1].velocity_set(0);
                wheels[2].velocity_set(0);
                wheels[3].velocity_set(0);
        } else if(!action_timer.is_running()) {
		/* Lado izquierdo */
    		wheels[0].velocity_set(side_factor[0]*velocity);
     		wheels[1].velocity_set(side_factor[0]*velocity);
		/* Lado derecho */
       		wheels[2].velocity_set(side_factor[1]*velocity);
        	wheels[3].velocity_set(side_factor[1]*velocity);
	}
	return true;
}

bool Navig::crab_move(short velocity)
{
	bool left = false;

	wheels[0].angle_set(90);
        wheels[1].angle_set(90);
        wheels[2].angle_set(90);
        wheels[3].angle_set(90);

	if(velocity < 0) {
		left = true;
		velocity = -velocity;
	}
	
	if(action != CRAB_MOVE) {
		action = CRAB_MOVE;
		action_timer.start(500);
	} else if(!action_timer.is_running()) {
		if(left) {
        		wheels[0].velocity_set(-velocity);
        		wheels[1].velocity_set(velocity);
        		wheels[2].velocity_set(velocity);
        		wheels[3].velocity_set(-velocity);
		} else {
        	        wheels[0].velocity_set(velocity);
               		wheels[1].velocity_set(-velocity);
 	        	wheels[2].velocity_set(-velocity);
 	        	wheels[3].velocity_set(velocity);
		}
	}

	return true;
}

bool Navig::stop()
{
	wheels[0].velocity_set(0);
        wheels[1].velocity_set(0);
        wheels[2].velocity_set(0);
        wheels[3].velocity_set(0);
	return true;
}

Navig::Navig()
{
	int i, ret = v3_open();
	if(ret == -1)
		cout << "Can't open V3 controller" << endl;
	action = -1;
}
