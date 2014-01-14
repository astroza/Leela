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
#include <timer.h>
#include <automata.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern "C" {
#include <v4.h>
}

using namespace cv;
using namespace std;
static Navig navig;
static Eye eye;
static NavigScript navigscript;
static int scenary_data = 0;

class MainAutomata : public Automata {
	public:
		int do_step();
};

int MainAutomata::do_step()
{
        Mat src(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)eye.rgb24_buf);
	Mat line_mask;
	static int test_threshold = 0;
	static Timer timer;

	AUTOM_BEGIN
	cvtColor(src, line_mask, CV_BGR2GRAY);
        threshold(line_mask, line_mask, test_threshold, 255, THRESH_BINARY_INV);
	cvtColor(line_mask, src, CV_GRAY2BGR);
	cout << "TEST THRESHOLD: " << test_threshold << endl;

	if(!timer.is_running()) {
		if(test_threshold >= 255)
			test_threshold = 0;
		else
			test_threshold+=5;
		timer.start(1000);
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
	int debug_enabled, ret;

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
	Chicken::init_dynamic_calibration(&eye);
        do {
            	tv.tv_sec = 5;
                tv.tv_usec = 0;
                eye.frame_wait_for(&tv);
                eye.frame_get(NULL, NULL);
		ret = control.do_step();
                if(debug)
                        debug->image_try_to_feed(eye.rgb24_buf, IMAGE_SIZE);
        } while(ret != -1);

        return 0;
}
