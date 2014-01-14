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
#include "aie.h"
#include "utfsm.h"
#include <timer.h>
#include <automata.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <climits>

extern "C" {
#include <v4.h>
#include <depth_sensor.h>
#include <mouse.h>
}

#define COLUMN_JUMPS            15
#define ROW_JUMPS               15
#define GRAY_THRESHOLD          40
#define LINE_POINTS		

using namespace cv;
using namespace std;
static Navig navig;
static Eye eye;
static NavigScript navigscript;



class MainAutomata : public Automata {
        private:
		bool fin;
                Mat src;
        public:          
                int do_step();
                MainAutomata();
}; 

MainAutomata::MainAutomata()
{
	src = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)eye.rgb24_buf);
	fin = false;
	depth_sensor_open();
}

int MainAutomata::do_step()
{
	static Timer timer;
	static Point dst;
        int x, y, i, pos;
	float rudder_value;
        Mat gray_frame;
	unsigned int v;
	vector<vector<Point> > pieces;
	static double start_pos;
	struct border_info binfo;
	static int confirmed = 0;
	Point top_point;
	Point g_point;
	static int fixing_try = 0;
	bool azul_present;
	static bool started = false;

	AUTOM_BEGIN

	if(!started) {
		timer.start(60000);
		started = true;
	}

	v = depth_sensor_read();
	cout << "V:" << v << endl;
#if 0
	cout << "YPOS:" << mouse_y_pos << endl;
	cout << "XPOS:" << mouse_x_pos << endl;
#endif

	Rescue::road_find(src, pieces, azul_present);
	if(pieces.size() > 0) {
		Rescue::get_border_info(src, pieces, binfo);

		
		cout << "DISTANCE: " << binfo.distance << endl;
		cout << "V SIZE: " << binfo.v_line_size << endl;
		cout << "V ANGLE" << binfo.v_angle << endl;

		if(binfo.v_angle > 127 && binfo.v_line_size > 22) {
			WAIT_WHILE(navig.rotate_with_angle(-7));
			reset();
			return 0;
		}

		if(binfo.distance > 240 || binfo.v_line_size < 22) {
			if(fixing_try > 2) {
				fixing_try = 0;
				WAIT_WHILE(navig.rotate_with_angle(30));
			} else {
				WAIT_WHILE(navig.rotate_with_angle(15));
				start_pos = mouse_y_pos;
				WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 2);
				navig.stop();
				WAIT_WHILE(navig.rotate_with_angle(-18));
				start_pos = mouse_y_pos;
				WAIT_WHILE(navig.forward(-30, 0.47) && (start_pos - mouse_y_pos) < 3.5);
				navig.stop();
				fixing_try++;
				reset();
				return 0;
			}
		}

		fixing_try = 0;

		start_pos = mouse_y_pos;
		WAIT_WHILE(Rescue::forward_road2(src, &navig, 30, azul_present) && (mouse_y_pos - start_pos) < 2);
		Rescue::road_find(src, pieces, azul_present);
		cout << "AZUL: " << azul_present << endl;
		navig.stop();

		if(!timer.is_running() && azul_present) {
			start_pos = mouse_y_pos;
                WAIT_WHILE(Rescue::forward_road2(src, &navig, 30, azul_present) && (mouse_y_pos - start_pos) < 10);
			navig.stop();
			exit(0);
		}

		if(pieces.size() > 0) {
			Rescue::get_border_info(src, pieces, binfo);
			cout << "AREA: " << contourArea(pieces[0]) << endl;
			top_point = Rescue::point_find_nearest(pieces[0], Point(0, 0));
			cout << "TOP POINT Y: " << top_point.y << endl; 
			if(top_point.y > 40 && pieces.size() == 1) {
				start_pos = mouse_y_pos;
                        	WAIT_WHILE(navig.forward(-30, 0.47) && (start_pos - mouse_y_pos) < 3.5);
				WAIT_WHILE(navig.rotate_with_angle(-30));
				start_pos = mouse_y_pos;
                        	WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 2);
				WAIT_WHILE(navig.rotate_with_angle(-30));
                                start_pos = mouse_y_pos;
                                WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 2);
				WAIT_WHILE(navig.rotate_with_angle(-30));
                                start_pos = mouse_y_pos;
                                WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 2);
			}

			if(pieces.size() == 2) {
				g_point	=  Rescue::point_find_nearest(pieces[1], Point(320, 240));
				if(g_point.x < 170) { // Contorno verde al lado
                                	start_pos = mouse_y_pos;
                                	WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 10);
					navig.stop();
					WAIT_WHILE(navig.rotate_with_angle(70));
					WAIT_WHILE(navig.forward(30, 0.53) && (mouse_y_pos - start_pos) < 10);
					 navig.stop();
				}
			}
		}
	}
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
        int frames_to_drop = 1;

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
	navigscript.init(&navig);
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

        } while(ret != -1);

        return 0;
}
