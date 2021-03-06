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
#include <navig.h>
#include <navig_script.h>
#include "debug.h"
#include "eye.h"
#include "utfsm14.h"
#include <timer.h>
#include <automata.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <climits>

extern "C" {
#include <v3.h>
#include <depth_sensor.h>
}

using namespace cv;
using namespace std;
static Navig navig;
static Eye eye;
static NavigScript navigscript;


class MainAutomata : public Automata {
	private:
		Mat src;
	public:
		int do_step();
		MainAutomata();
};

MainAutomata::MainAutomata()
{
	src = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)eye.rgb24_buf);
}


int MainAutomata::do_step()
{
	static Timer timer;
	static Aligner aligner(&navig);
	static int prev_wall_distance;
	float wall_distance = (wall_sensor0_distance() + wall_sensor1_distance())/2.0;
	static int vel;
	static bool rotate = false;
	static bool (*obstacle_in_side)();

	cout << obstacle_in_right_side() << endl << obstacle_in_left_side << endl;
	AUTOM_BEGIN
	JUMP(&aligner);

	WAIT_WHILE(navig.forward(20, 1.0) && !obstacle_in_front());
	navig.stop();
	prev_wall_distance = wall_distance;

	if(rotate) {
		rotate = false;
		timer.start(1000);
		WAIT_WHILE(navig.forward(-50, 1.0) && timer.is_running());
		navig.stop();
		timer.start(1200);
		WAIT_WHILE(navig.rotate(-50) && timer.is_running());
		reset();
		return 0;
	}

	JUMP(&aligner);
	/* Esquivando obstaculo */
	if(wall_distance >= 50) {
		vel = 20;
		obstacle_in_side = obstacle_in_right_side;
	} else {
		vel = -20;
		obstacle_in_side = obstacle_in_left_side;
	}

	WAIT_UNTIL(navig.crab_move(vel) && !obstacle_in_side());
	timer.start(800);
	WAIT_WHILE(navig.crab_move(vel) && timer.is_running());
	navig.stop();

	timer.start(2500);
	WAIT_WHILE(navig.forward(50, 1.0) && timer.is_running());
	navig.stop();

	JUMP(&aligner);
	WAIT_UNTIL(navig.crab_move(-vel) && wall_distance <= prev_wall_distance);
	navig.stop();
	rotate = true;

	reset();
	AUTOM_END

	return 0;
}

int main(int c, char **v)
{
	AutomataControl control;
	MainAutomata main_automata;
        Debug *debug;
        struct timeval tv;
	int debug_enabled, ret = 0;
        int dropped_frames = 0;
        int frames_to_drop = 0;

        if(c < 2) {
                printf("%s <debug>\n", v[0]);
                return 0;
        }

        debug_enabled = strtol(v[1], NULL, 10);

	if(debug_enabled > 0)
        	debug = new Debug(&eye);
	else
		debug = NULL;

	eye.cam_open("/dev/video0");
        eye.stream_on();
	control.jump(&main_automata);
	navigscript.init(&navig, NULL);
	v3_analog_inputs_enable(V3_A0|V3_A1|V3_A2|V3_A3);

	v3_work();
        do {
            	tv.tv_sec = 5;
                tv.tv_usec = 0;
                eye.frame_wait_for(&tv);
                eye.frame_get(NULL, NULL);
                if(dropped_frames >= frames_to_drop) {
                        ret = control.do_step();
                        if(debug)
                                debug->image_try_to_feed(eye.rgb24_buf, IMAGE_SIZE);
                        dropped_frames = 0;
                } else
                      	dropped_frames++;
		v3_work();
        } while(ret != -1);

        return 0;
}
