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
	unsigned char *px;
	static unsigned char min[3]={255, 255, 255};
	static unsigned char max[3]={0, 0, 0};
	int i;

	AUTOM_BEGIN

	px = (unsigned char *)&eye.rgb24_buf[(320*120+160)*3];
	for(i = 0; i < 3; i++) {
		if(min[i] > px[i])
			min[i] = px[i];
		if(max[i] < px[i])
			max[i] = px[i];
	}
	if(Debug::is_enabled())
		circle(src, Point(160, 120), 10, Scalar(255, 0, 0), 5);

	printf("MIN=(%u, %u, %u)\n", min[0], min[1], min[2]);
	printf("MAX=(%u, %u, %u)\n", max[0], max[1], max[2]);
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
