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
	navigscript.init(&navig, &picker);
	Chicken::init_dynamic_calibration(&eye);
        do {
            	tv.tv_sec = 5;
                tv.tv_usec = 0;
                eye.frame_wait_for(&tv);
                eye.frame_get(NULL, NULL);
		ret = control.do_step();
                if(debug)
                        debug->image_try_to_feed(eye.rgb24_buf, IMAGE_SIZE);
		v3_work();
        } while(ret != -1);

        return 0;
}
