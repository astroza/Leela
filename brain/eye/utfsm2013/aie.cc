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
#include "aie.h"
#include "debug.h"
#include <math.h>
#include <sstream>
#include <assert.h>

using namespace cv;
using namespace std;

#define HALF_DISTANCE 80
#define MAX_LINE_WIDTH 60
#define THRESHOLD_CONFIG_FILENAME "thresholds.config"

DRange::DRange(double min, double max)
{
	this->min = min;
	this->max = max;
}

bool DRange::contains(double x)
{
	if(x >= min && x <= max)
		return true;
	return false;
}


void DRanges::add(const DRange &range)
{
	ranges.push_back(range);
}

bool DRanges::contains(double x)
{
	vector<DRange>::iterator it;

	for(it = ranges.begin(); it < ranges.end(); it++)
		if((*it).contains(x))
			return true;
	return false;
}

LineColor::LineColor(const Scalar &min, const Scalar &max)
{
	this->range[0] = min;
	this->range[1] = max;
}

GLine::GLine(const Point &r_A, const Point &r_B)
{
	this->r_A = r_A;
	this->r_B = r_B;
	reset();
}

VLine::VLine() : GLine(Point(0, 240), Point(0, 0))
{

}

void VLine::digest(Point &p)
{
	if(p.y <= A.y)
		A = p;
	if(p.y >= B.y)
		B = p;
	used = true;
}

HLine::HLine() : GLine(Point(320, 0), Point(0, 0))
{

}

void HLine::digest(Point &p)
{
	if(p.x <= A.x)
		A = p;
	if(p.x >= B.x)
		B = p;
	used = true;
}

void GLine::reset()
{
	A = r_A;
	B = r_B;
	used = false;
}

Point GLine::get_A()
{
	return A;
}

Point GLine::get_B()
{
	return B;
}

double GLine::get_angle()
{
	return atan2(B.y - A.y, B.x - A.x);
}

double GLine::get_length()
{
	return sqrt(pow(B.y-A.y, 2.0) + pow(B.x - A.x, 2.0));
}

bool GLine::is_used()
{
	return used;
}

bool GLine::equals(GLine &l)
{
	if(l.get_A() == A && l.get_B() == B)
		return true;
	return false;
}

int Position::directions[4][2] = { {1, 0}, {0, -1}, {-1, 0}, {0, 1}};
Position::Position()
{
	this->current_point.x = 0;
	this->current_point.y = 0;
	this->current_direction = 0;
}
/*
int Position::get_rotation_sign_for(int final_direction)
{
	int d = final_direction - current_direction;

	if(abs(d) > 2)
		return -(d/abs(d));

	return (d/abs(d));
}
*/

int Position::get_rotation_sign_for(int final_direction)
{
	int d = final_direction - current_direction;
	switch(d) {
		case 3:
			return 1;
		case 2:
		case 1:
			return -1;
		case -1:
		case -2:
			return 1;
		case -3:
			return -1;
		default:
			exit(-1);
	}
}

int Position::get_rotation_sign_for(Point dst_point)
{
	return get_rotation_sign_for(find_direction_for(dst_point));
}

int Position::find_direction_for(Point dst_point)
{
	int d_row = dst_point.y - current_point.y;
	int d_column = dst_point.x - current_point.x;
	static const int dir_map[] = {1, 2, 0, 0, 3};

	if(d_row != 0)
		d_row = d_row / abs(d_row);
	if(d_column != 0)
		d_column = d_column / abs(d_column);
	return dir_map[d_row + d_column*2 + 2];
}

Point Position::get_point()
{
	return current_point;
}

void Position::advance()
{
	int next_row = current_point.y + directions[current_direction][0];
	int next_column = current_point.x + directions[current_direction][1];
#if 0
	cout << "NEXT ROW: " << next_row << endl;
	cout << "NEXT COLUMN: " << next_column << endl;
#endif
	assert(next_row < GRID_ROWS && next_row >= 0);
	assert(next_column < GRID_COLUMNS && next_column >= 0);

	this->current_point.y = next_row;
	this->current_point.x = next_column;
}

namespace Chicken {

#define BINARY_THRESHOLD 90

	static LineColor black_line(Scalar(0, 0, 0), Scalar(180, 255, 70));
	static Eye *eye_calib = NULL;
	static int *thresholds;
	static int thresholds_count;
	static int current_threshold = 0;

	void init_dynamic_calibration(Eye *eye)
	{
		ifstream thresholds_config;
		int i;

		eye_calib = eye;

		thresholds_config.open(THRESHOLD_CONFIG_FILENAME, ifstream::in);
		thresholds_config >> thresholds_count;
			thresholds = new int[thresholds_count];
		for(i = 0; i < thresholds_count; i++)
			thresholds_config >> thresholds[i];
		thresholds_config.close();
	}

	bool find_road_contour(Mat &orig_frame, vector<Point> &points, double gray_threshold, double &max_area)
	{
		unsigned int i, cmax = 0;
		vector<vector<Point> > contours;
		Mat line_mask;

#if 0
		cout << "TESTING THRESHOLD: " << gray_threshold << endl;
#endif
		cvtColor(orig_frame, line_mask, CV_BGR2GRAY);
		threshold(line_mask, line_mask, gray_threshold, 255, THRESH_BINARY_INV);

		findContours(line_mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		if(contours.size() > 0) {
        		max_area = contourArea(contours[0]);
                	for(i = 1; i < contours.size(); i++) {
                		double area = contourArea(contours[i]);
                     	  	if(area > max_area) {
                       	        	cmax = i;
                                	max_area = area;
                        	}
                	}
                	/* Rendimiento: En caso de obtener pocos frames, sacar esta aproximacion */
                	approxPolyDP(contours[cmax], contours[cmax], 5.0, true);

                	points = contours[cmax];
			return true;
		}
		return false;
	}

	template <class Line>
	static unsigned int find_max_length(vector<Line> &lines)
	{
		unsigned int i;
		unsigned int i_max = -1;
		double len_max = 0;
		double l;

		for(i = 0; i < lines.size(); i++) {
			l = lines[i].get_length();
			if(l > len_max) {
				len_max = l;
				i_max = i;
			}
		}
		return i_max;
	}

	static bool find_road_line(vector<Point> &points, VLine &neg_line, VLine &pos_line, DRanges &ranges)
	{
		double angle;
		unsigned int i, neg_max, pos_max;
                vector<VLine> neg_lines;
                vector<VLine> pos_lines;

		if(points.size() < 3)
			return false;

        	points.push_back(points.front());
		for(i = 0; i+1 < points.size(); i++) {
			angle = atan2(points[i+1].y - points[i].y, points[i+1].x - points[i].x);
			if(ranges.contains(abs(angle))) {
                                /* Linea negativa */
                                if(angle < 0) {
                                        neg_line.digest(points[i]);
                                        neg_line.digest(points[i+1]);
                                } else {
                                        pos_line.digest(points[i]);
                                        pos_line.digest(points[i+1]);
                                }
                        } else if(neg_line.is_used()) {
                                neg_lines.push_back(neg_line);
                                neg_line.reset();
                        } else if(pos_line.is_used()) {
                                pos_lines.push_back(pos_line);
                                pos_line.reset();
                        }
                }
		points.pop_back();
                if(neg_line.is_used())
                                neg_lines.push_back(neg_line);
                if(pos_line.is_used())
                                pos_lines.push_back(pos_line);

                if(neg_lines.size() == 0 || pos_lines.size() == 0)
                        return false;

                neg_max = find_max_length<VLine>(neg_lines);
                pos_max = find_max_length<VLine>(pos_lines);
		neg_line = neg_lines[neg_max];
		pos_line = pos_lines[pos_max];
		return true;
	}

	bool detect_T(Mat &orig_frame, vector<Point> &points, double *distance)
	{
		unsigned int i;
		double angle;
		DRanges ranges;
		vector<HLine> lines;
		vector<HLine>::iterator it;
		HLine hline;
		Point A;

		ranges.add(DRange(-0.2*M_PI, 0.2*M_PI));
		ranges.add(DRange(0.8*M_PI, M_PI));
		ranges.add(DRange(-M_PI, -0.8*M_PI));

                if(points.size() < 3)
                        return false;

                points.push_back(points.front());
                for(i = 0; i+1 < points.size(); i++) {
                        angle = atan2(points[i+1].y - points[i].y, points[i+1].x - points[i].x);
                        if(ranges.contains(abs(angle))) {
				hline.digest(points[i]);
				hline.digest(points[i+1]);
			} else if(hline.is_used()) {
#if 0
				cout << hline.get_length() << endl;
#endif
				if(hline.get_length() > MAX_LINE_WIDTH) {
					lines.push_back(hline);
					if(Debug::is_enabled()) {
						line(orig_frame, hline.get_A(), hline.get_B(), Scalar(0, 100, 150), 5);
					}
				}
				hline.reset();
			}
		}

		if(lines.size() == 0)
			return false;

		A.x = 0;
		A.y = 0;
		for(it = lines.begin(); it < lines.end(); it++) {
			A.x += (*it).get_A().x + (*it).get_B().x;
			A.y += (*it).get_A().y + (*it).get_B().y;
		}

		A.x /= lines.size()*2;
		A.y /= lines.size()*2;

		points.pop_back();
                if(Debug::is_enabled()) {
			circle(orig_frame, A, 10, Scalar(0, 0, 255), 5);
		}

		if(distance != NULL)
			*distance = (float)(240 - A.y);

		return true;
	}

	bool get_next_direction(Mat &orig_frame, vector<Point> &points, VLine &neg_line, VLine &pos_line, VLine &dir)
	{
		DRanges ranges;
		Point A, B, C;

		neg_line = VLine();
		pos_line = VLine();
		ranges.add(DRange(M_PI*0.25, M_PI*0.75));
		if(!find_road_line(points, neg_line, pos_line, ranges))
			return false;

		A = neg_line.get_A();
		B = pos_line.get_B();
#if 0
		cout << "ANGLE DIFF: " << ((pos_line.get_angle() - neg_line.get_angle())/M_PI*180) << endl;
#endif
		/* Para aca me debo dirigir */
		C.x = (A.x + B.x)/2.0;
		C.y = (A.y + B.y)/2.0;
		dir.reset();
		Point bottom(160, 240);
		dir.digest(bottom);
		dir.digest(C);
		
		return true;
	}

	static bool need_threshold_calibration(bool find_ret, unsigned int points_size, double area, VLine &bottom_top)
	{
#if 0
		int top_bottom_distance = bottom_top.get_B().y - bottom_top.get_A().y;
                cout << "TOP BOTTOM DISTANCE: " << top_bottom_distance << endl;
                cout << "AREA: " << area << endl;
                cout << "CONTOUR SIZE: " << points_size << endl;
                cout << "BOTTOM POINT: " << bottom_top.get_B().y << endl;
#endif
		if(find_ret == false || points_size < 4 || area < 4000) {
			return true;
		} else if((points_size > 15 && area > 15000) || area > 70000) {
			return true;
		}

		return false;
	}

	bool forward_line(Navig *navig, Mat &src, vector<Point> &points) 
	{
		VLine dir;
                float rudder_value;
                double rudder_angle;

		if(Chicken::get_next_direction(navig, src, points, dir) == false)
			return false;
                /* Correccion suave? */
                rudder_angle = dir.get_angle()/M_PI*180;
		rudder_value = rudder_angle/180;
#ifdef DEBUG
		cout << "!!!!RUDDER ANGLE=" << rudder_angle << endl;
		cout << "!!!!RUDDER VALUE=" << rudder_value << endl;
#endif
                navig->forward(100, rudder_value);

                return true;
	}

	bool align_to_line(Navig *navig, Mat &src, vector<Point> &points)
	{
                DRanges ranges;
                VLine neg_line = VLine();
                VLine pos_line = VLine();
		double angle_average, rudder_angle;

                ranges.add(DRange(M_PI*0.25, M_PI*0.75));
                if(!find_road_line(points, neg_line, pos_line, ranges))
			return true;

		angle_average = (neg_line.get_angle() + pos_line.get_angle()) / 2.0;
		
		rudder_angle = angle_average/M_PI*180;
		cout << "RUDDER ANGLE:" << rudder_angle << endl;
		exit(0);
		if(abs(rudder_angle - 90) >= 15) {
#if 0
			cout << "ANGLE: " << rudder_angle << endl;
#endif
			navig->rotate(rudder_angle > 90? 30 : -30);
			return false;
		} else {
			navig->stop();
			return true;
		}
	}

	bool align_to_line(Navig *navig, Mat &src)
	{
		vector<Point> points;

		return align_to_line(navig, src, points);
	}

	bool get_next_direction(Navig *navig, Mat &src, vector<Point> &points, VLine &dir)
	{
		double area;
		bool find_ret;
		vector<Point>::iterator it;
		VLine bottom_top;
		bool need_calibration = false;
		int best_threshold = 0;
		int calib_step = 0;
		double angle_diff;
		double max_area = 0;
		VLine pos_line, neg_line;

#if 0
		do {
#endif
			//cout << "THRESHOLD: " << thresholds[current_threshold] << endl;
        		find_ret = Chicken::find_road_contour(src, points, thresholds[current_threshold], area);
			if(Chicken::get_next_direction(src, points, neg_line, pos_line, dir) == false)
				return false;

			angle_diff = (pos_line.get_angle() - neg_line.get_angle())/M_PI*180;
#if 0
			if(angle_diff > 4)
				find_ret = false;

			for(it = points.begin(); it != points.end(); it++)
				bottom_top.digest(*it);
				/* bottom_top.get_A() es el punto mas alto y bottom_top.get_B() el mas bajo */

			switch(calib_step) {
				case 0:
					need_calibration = need_threshold_calibration(find_ret, points.size(), area, bottom_top);
					if(need_calibration) {
						navig->stop();
						current_threshold = 0;
						calib_step++;
						/* Sin break, necesito q se ejecute el siguiente paso de inmediato */
					} else
						break;
				case 1:
					if(!need_threshold_calibration(find_ret, points.size(), area, bottom_top))
						if(area > max_area) {
							max_area = area;
							best_threshold = current_threshold;
						}

					if(current_threshold + 1 < thresholds_count)
						current_threshold++;
					else {
						current_threshold = best_threshold;
						calib_step++;
					}
					break;
				case 2:
					cout << "CALIBRATION RESULT: " << thresholds[current_threshold] << endl;
					need_calibration = false;
			}
		} while(need_calibration);
#endif

		Chicken::get_next_direction(src, points, neg_line, pos_line, dir);
                if(Debug::is_enabled()) {
			vector<vector<Point> > contours;
			double angle;
                	ostringstream convert;
                	string angle_text;

			contours.push_back(points);
                	drawContours(src, contours, 0, Scalar(0, 199, 224), CV_FILLED);

                        line(src, neg_line.get_A(), neg_line.get_B(), Scalar(255, 0, 0), 5);
                        angle = neg_line.get_angle();
                        convert << (int)(angle/M_PI*180);
                        angle_text = convert.str();
                        convert.str("");
                        putText(src, angle_text, Point(250, 120), FONT_HERSHEY_PLAIN, 1.2, Scalar(255, 0, 0), 2);
                        line(src, pos_line.get_A(), pos_line.get_B(), Scalar(0, 255, 0), 5);
                        angle = pos_line.get_angle();
                        convert << (int)(angle/M_PI*180);
                        angle_text = convert.str();
                        putText(src, angle_text, Point(10, 120), FONT_HERSHEY_PLAIN, 1.2, Scalar(0, 255, 0), 2);
                        line(src, dir.get_A(), dir.get_B(), Scalar(200, 200, 0), 3);
		}
		return true;
	}

	bool forward_Tless_line(Navig *navig, Mat &src)
	{
		vector<Point> points;
		double T_distance;
		static bool inhibited = true;

		if(forward_line(navig, src, points) == false)
			return false;

        	if(Chicken::detect_T(src, points, &T_distance)) {
                	cout << "T distance: " << T_distance << endl;
			if(T_distance >= HALF_DISTANCE)
				inhibited = false;

                	if(!inhibited && T_distance <= 41) {
				cout << "DETECT T" << endl;
                        	navig->stop();
				inhibited = true;
				return false;
			}
                }
		return true;
	}

	bool forward_line(Navig *navig, Mat &src)
	{
		vector<Point> points;
		return forward_line(navig, src, points);
	}
};
