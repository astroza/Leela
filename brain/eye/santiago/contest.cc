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

#define TIME_NEEDED_TO_PASS_T 240

extern "C" {
#include <v3.h>
}

int current_point = 0;
vector<int> marks;

using namespace cv;
using namespace std;
static Navig navig;
static Picker picker;
static Eye eye;
static NavigScript navigscript;

class Contest {
	private:
		vector<Point> points_to_visit;
		vector<Point> shortest_path;
		int min_path_size;
		vector<Point>::iterator it;
	public:
		Position position;
		bool read_file(const char *contest_file);
		void build_shortest_path();
		void build_shortest_path(vector<Point> &path, Point start, int cost);
		void print_shortest_path();
		bool next_point(Point &next);
		Contest() { min_path_size = INT_MAX; }
};

/* Hack para evitar bug de firmware en Flyduino */
static int mark_state = 3;
void mark_set(int enable)
{
	if(enable == 1) {
		//cout << "TURN ON LIGHT" << endl;
		mark_state = 0;
	} else if(mark_state == 0) {
		//cout << "TURN OFF LIGHT" << endl;
		mark_state = 1;
	}
}

class MainAutomata : public Automata {
	private:
		Contest contest;
		Mat src;
	public:
		int do_step();
		MainAutomata();
};

class DirectionFixer : public Automata {
	private:
		Timer timer;
		Position *position;
		Point dst;
		Mat *orig_frame;
	public:
		int do_step();
		DirectionFixer(Mat *orig_frame, Position *position, Point &dst);
		DirectionFixer();
};

MainAutomata::MainAutomata()
{
	contest.read_file("contest.in");
	contest.build_shortest_path();
	src = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)eye.rgb24_buf);
}

int DirectionFixer::do_step()
{
	static int rotation_sign;

	AUTOM_BEGIN
        WAIT_UNTIL(Chicken::align_to_line(&navig, *orig_frame));
        navig.stop();
#if 0
	cout << "CUR DIRECTION: " << position->get_direction() << endl;
	cout << "DESIRED DIRECTION: " << position->find_direction_for(dst) << endl;
#endif
	if(position->get_direction() == position->find_direction_for(dst))
		return -1;

	rotation_sign = position->get_rotation_sign_for(dst);
	navig.rotate(0);
	WAIT_UNTIL(navig.ready_to_rotate());
	timer.start(500);
	WAIT_WHILE(navig.rotate(rotation_sign * 100) && timer.is_running());
	position->set_direction(position->get_direction() + -1*rotation_sign);
	navig.stop();
	reset();
	AUTOM_END

	return 0;
}

DirectionFixer::DirectionFixer(Mat *orig_frame, Position *position, Point &dst)
{
	this->position = position;
	this->orig_frame = orig_frame;
	this->dst = dst;
}

DirectionFixer::DirectionFixer()
{
}

int MainAutomata::do_step()
{
	static Timer timer;
	static Point dst;
	static DirectionFixer dfixer;

	AUTOM_BEGIN

	if(!contest.next_point(dst)) {
		mark_set(1);
		WAIT_WHILE(true);
	}
#if 1
	cout << "CURRENT POINT: ROW=" << dst.y << " COLUMN=" << dst.x << "MARK: " << marks[current_point] << endl;
#endif
	dfixer = DirectionFixer(&src, &contest.position, dst);
	if(marks[current_point] == 1) {
		mark_set(true);
	}
	JUMP(&dfixer);
	mark_set(0);
	navig.stop();
	WAIT_WHILE(Chicken::forward_Tless_line(&navig, src));
	timer.start(TIME_NEEDED_TO_PASS_T);
	WAIT_WHILE(navig.forward(100, 1.0) && timer.is_running());
	navig.stop();
	contest.position.advance();
	current_point++;
	reset();
	AUTOM_END

	return 0;
}

bool Contest::read_file(const char *contest_file)
{
	ifstream contest_in;
	char row;
	int column;
	int direction;
	int points_count;

	contest_in.open(contest_file, ifstream::in);
	if(!contest_in.is_open())
		return false;

	/* Formato:
	 * <fila inicial> <columna inicial> <direccion inicial>
	 * <numero de puntos a visitar>
	 * <primer punto>
	 * ....
	 * <ultimo punto>
         * Cada punto es un una <fila> <columna>
	 */
	contest_in >> row >> column >> direction >> points_count;
	assert((row-'A') >= 0);
	assert(column-1 >= 0);
	position.set_row(row-'A');
	position.set_column(column-1);
	//position.set_row(column-1);
	//position.set_column(row-'A');
	position.set_direction(direction);
	while(points_count--) {
		contest_in >> row >> column;
        	assert((row-'A') >= 0);
        	assert(column-1 >= 0);
		points_to_visit.push_back(Point(column-1, row-'A'));
		//points_to_visit.push_back(Point(row-'A', column-1));
	}

	contest_in.close();
	return true;
}

void Contest::print_shortest_path()
{
	vector<Point>::iterator it;
	int i = 0;

	for(it = shortest_path.begin(); it != shortest_path.end(); it++) {
		cout << "( " << (char)((*it).y+'A') << ", ";
		cout << ((*it).x + 1) << ")";
		if(marks[i] == 1)
			cout << " MARK";
		i++;
		cout << endl;
	}
}

void Contest::build_shortest_path()
{
	vector<Point> path;
	Point start;
	int delta, step;
	unsigned int i;

	start = position.get_point();
	build_shortest_path(path, start, 0);
	path.clear();
	path.push_back(start);
	marks.push_back(0);
	for(i = 0; i < shortest_path.size(); i++) {
		while( abs((delta = shortest_path[i].x - path.back().x)) > 0) {
			step = delta/abs(delta);
			path.push_back(Point(path.back().x+step, path.back().y));
			marks.push_back(0);
		}

                while( abs((delta = shortest_path[i].y - path.back().y)) > 0) {
                        step = delta/abs(delta);
                        path.push_back(Point(path.back().x, path.back().y+step));
			marks.push_back(0);
                }
		marks.pop_back();
		marks.push_back(1);
	}
	shortest_path = path;
	shortest_path.erase(shortest_path.begin());
#if 1
	print_shortest_path();
#endif
	it = shortest_path.begin();
}

void Contest::build_shortest_path(vector<Point> &path, Point start, int cost)
{
	vector<Point>::iterator it;
	Point dst;

	if(points_to_visit.size() == 0) {
		if(cost < min_path_size) {
			min_path_size = cost;
			shortest_path = path;
#if 1
			cout << "SHORTEST SIZE: " << cost << endl;
#endif
		}
		return;
	}

	for(it = points_to_visit.begin(); it != points_to_visit.end(); it++) {
		dst = *it;
		points_to_visit.erase(it);
		path.push_back(dst);
		build_shortest_path(path, dst, cost + abs(dst.x-start.x) + abs(dst.y-start.y));
		path.pop_back();
		points_to_visit.insert(it, dst);
	}
}

bool Contest::next_point(Point &next)
{
	if(it == shortest_path.end()) {
		return false;
	}

	next = *it;
	it++;
	return true;
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

		if(mark_state < 3) {
			switch(mark_state) {
				case 0:
					v3_servo_set(8, 180);
					break;
				case 1:
					v3_servo_set(8, 0);
					mark_state++;
					break;
				case 2:
					v3_servo_disconnect(8);
					mark_state++;
					break;
			}
		}
		v3_work();
        } while(ret != -1);

        return 0;
}
