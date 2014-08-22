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
	pieza_read_min_max();
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
	bool pieza_present;
	vector<Point> pieza;
	bool azul;
	AUTOM_BEGIN

	v = depth_sensor_read();
	cout << "V:" << v << endl;
#if 0
	cout << "YPOS:" << mouse_y_pos << endl;
	cout << "XPOS:" << mouse_x_pos << endl;
#endif
	cout << "PIEZA AZUL:" << pieza_present << endl;
	Rescue::road_find(src, pieces, azul);
	if(pieces.size() > 0) {
		Rescue::get_border_info(src, pieces, binfo);

		
		cout << "DISTANCE: " << binfo.distance << endl;
		cout << "V SIZE: " << binfo.v_line_size << endl;
		cout << "V ANGLE" << binfo.v_angle << endl;
		cout << "PIECES COUNT: " << pieces.size() << endl;
		cout << "AREA: " << contourArea(pieces[0]) << endl;
		top_point = Rescue::point_find_nearest(pieces[0], Point(0, 0));
		cout << "TOP POINT Y: " << top_point.y << endl; 
		cout << "AZUL: " << azul << endl;

		

/* // CODIGO VIEJO
	if(pieces.size() > 2 && binfo.v_line_size <= 218) {
		start_pos = mouse_y_pos;
		WAIT_WHILE(Rescue::forward_road2(src, &navig, 30) && (mouse_y_pos - start_pos) < 60);
		WAIT_WHILE(navig.rotate_with_angle(70));
		start_pos = mouse_y_pos;
		WAIT_WHILE(navig.forward(50, 0.5) && (mouse_y_pos - start_pos) < 15);
	} else if(pieces.size() <= 1 && binfo.v_line_size < 200) {
                start_pos = mouse_y_pos;
                WAIT_WHILE(Rescue::forward_road2(src, &navig, 20) && (mouse_y_pos - start_pos) < 15);
		WAIT_WHILE(navig.rotate_with_angle(-45));
		start_pos = mouse_y_pos;
		WAIT_WHILE(navig.forward(50, 0.5) && (mouse_y_pos - start_pos) < 5);
		navig.stop();
		WAIT_WHILE(navig.rotate_with_angle(-45));
		explored = false;
	} else {
		cout << "NORMAL" << endl;
		start_pos = mouse_y_pos;
		WAIT_WHILE(Rescue::forward_road(src, &navig, pieces, 50) && (mouse_y_pos - start_pos) < 5);
	}*/
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
