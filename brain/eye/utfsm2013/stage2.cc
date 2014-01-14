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
#include <climits>

extern "C" {
#include <v4.h>
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

int get_angle(Point p1, Point p2)
{
        return (atan2(p2.y-p1.y, p2.x-p1.x)/CV_PI)*180;
}

Point next_average_point(int *pos, int length, int average_points, Point *line_points)
{
        int count;
        Point ret;

        ret.x = 0;
        ret.y = 0;
        for(count = 0; *pos < length && count < average_points; count++, (*pos)++) {
                ret.x += line_points[*pos].x;
                ret.y += line_points[*pos].y;
        }

        if(count) {
                ret.x /= count;
                ret.y /= count;
        } else
                ret.x = -1;

        return ret;
}

int detect_T(Mat &debug, int pos, int length, Point *line_points)
{
        Point A, B;
        int angle[2];

        B = next_average_point(&pos, length, 15, line_points);
        A = next_average_point(&pos, length, 15, line_points);

        if(A.x == -1 || B.x == -1)
                return 0;

#ifdef DEBUG
        line(debug, A, B, Scalar(255,0,0), 3, 8, 0);
#endif
        angle[0] = get_angle(A, B);

        B = next_average_point(&pos, length, 15, line_points);
        A = next_average_point(&pos, length, 15, line_points);

        if(A.x == -1 || B.x == -1)
                return 0;
#ifdef DEBUG
        line(debug, A, B, Scalar(255,0,255), 3, 8, 0);
#endif
        angle[1] = get_angle(A, B);

	printf("angle[0]=%d angle[1]=%d abs(angle[0]-angle[1])=%d\n", angle[0], angle[1], abs(angle[0]-angle[1]));
        return abs(angle[0]-angle[1]) > 80;
}

class MainAutomata : public Automata {
        private:
		bool fin;
                Mat src;
        public:          
                int do_step();
                MainAutomata();
}; 

class DirectionFixer : public Automata {
        private:
                Mat *orig_frame;
		short next_angle;
        public: 
                int do_step();
                DirectionFixer(Mat *orig_frame, float last_rudder_value);
                DirectionFixer();
};

DirectionFixer::DirectionFixer(Mat *orig_frame, float last_rudder_value)
{
	this->orig_frame = orig_frame;
	this->next_angle = last_rudder_value < 0.5? -45 : 45;
}

DirectionFixer::DirectionFixer()
{
}

static int get_points_count(Mat *orig_frame)
{
        Mat gray_frame;
        static Point line_points[768];
        static int line_points_length;
	static int x, y;

        cvtColor(*orig_frame, gray_frame, CV_BGR2GRAY);
        threshold(gray_frame, gray_frame, GRAY_THRESHOLD, 255, THRESH_BINARY);
        line_points_length = 0;
        for(y = 234; y >= 0; y -= ROW_JUMPS) {
                for(x = 4; x < 320; x += COLUMN_JUMPS) {
                        if(gray_frame.at<unsigned char>(y, x) == 0) {
                                line_points[line_points_length].x = x;
                                line_points[line_points_length++].y = y;
                        }
                }
        }
	return line_points_length;
}
int DirectionFixer::do_step()
{
        AUTOM_BEGIN

	WAIT_WHILE(navig.rotate_with_angle(next_angle));
	if(get_points_count(orig_frame) != 0)
		return -1;
	WAIT_WHILE(navig.rotate_with_angle(next_angle*-2));
	return -1;

	AUTOM_END;
	return 0;
}

MainAutomata::MainAutomata()
{
	src = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)eye.rgb24_buf);
	fin = false;
}

int MainAutomata::do_step()
{
	static Timer timer;
	static Point dst;
	static double start_pos;
        int x, y, i, pos;
	float rudder_value;
	static float last_rudder_values[] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
	static int rudder_idx = 0;
        Mat gray_frame;
	Point Ap, Bp;
	static Point line_points[768];
	static int line_points_length;
	DirectionFixer dfixer;

	AUTOM_BEGIN

	cvtColor(src, gray_frame, CV_BGR2GRAY);
	threshold(gray_frame, gray_frame, GRAY_THRESHOLD, 255, THRESH_BINARY);
	line_points_length = 0;
	for(y = 234; y >= 0; y -= ROW_JUMPS) {
		for(x = 4; x < 320; x += COLUMN_JUMPS) {
			if(gray_frame.at<unsigned char>(y, x) == 0) {
				line_points[line_points_length].x = x;
				line_points[line_points_length++].y = y;
			}
		}
	}

	if(line_points_length == 0) {
		for(i = 0; i < 10; i++)
			rudder_value += last_rudder_values[i];
		rudder_value /= 10;
		if(rudder_value < 0.5)
			rudder_value = 0;
		else if(rudder_value > 0.5)
			rudder_value = 1.0;
		navig.forward(200, rudder_value);
	} else {

	pos = 0;
	Bp = next_average_point(&pos, line_points_length, line_points_length, line_points);
        Ap.x = 160;
        Ap.y = 239;

	if(Debug::is_enabled()) {
		for(i = 1; i < line_points_length; i++) {
			line(src, line_points[i-1], line_points[i], Scalar(100,100,100), 3, 8, 0);
		}
        	circle(src, Ap, 5, Scalar(0,255,255), -1, 8, 0);
        	circle(src, Bp, 5, Scalar(0,0,255), -1, 8, 0);
	}
        rudder_value = get_angle(Bp, Ap)/180.0;
	last_rudder_values[rudder_idx++%10] = rudder_value;
	if(rudder_value < 0.25)
		rudder_value = 0;
	if(rudder_value > 0.75)
		rudder_value = 1.0;
	//if(fin == false)
        	navig.forward(200, rudder_value);
		//cout << "forward" << endl;
	//else
	//	navig.stop();
/*
	if(detect_T(src, 0, line_points_length, line_points)) {
		navig.forward(30, rudder_value);
		start_pos = mouse_y_pos;
		WAIT_UNTIL(mouse_y_pos - start_pos >= 14);
		navig.stop();
		cout << "FIN" << endl;
		fin = true;
	}*/
	cout << "rudder=" << rudder_value << endl;
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
	navigscript.init(&navig);
        do {
            	tv.tv_sec = 5;
                tv.tv_usec = 0;
                eye.frame_wait_for(&tv);
                eye.frame_get(NULL, NULL);
                //if(dropped_frames >= frames_to_drop) {
                        ret = control.do_step();
                        if(debug)
                                debug->image_try_to_feed(eye.rgb24_buf, IMAGE_SIZE);
                        //dropped_frames = 0;
                //} else
                  //    dropped_frames++;

        } while(ret != -1);

        return 0;
}
