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
#include <v3.h>
}

using namespace cv;
using namespace std;
static Navig navig;
static Picker picker;
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
        vector<Point> points;

	AUTOM_BEGIN
	//Chicken::forward_Tless_line(&navig, src);
	//navig.stop();
	//WAIT_UNTIL(Chicken::align_to_line(&navig, src));
	//cout << "ALIGNED" << endl;
	return 0;
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
	navigscript.init(&navig, &picker);
	Chicken::init_dynamic_calibration(&eye);
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
