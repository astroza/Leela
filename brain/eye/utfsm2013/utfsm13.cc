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

RegionSensor::RegionSensor()
{
}

void RegionSensor::debug(Mat &orig_frame)
{
}

void 
namespace Stage2 {

#define BINARY_THRESHOLD 90

	static Eye *eye_calib = NULL;
	static int *thresholds;
	static int thresholds_count;
	static int current_threshold = 0;

#define COLUMN_JUMPS 		15
#define ROW_JUMPS		15
#define GRAY_THRESHOLD 		20

	void extract_points(Mat &orig_frame, Mat &points)
	{
		unsigned int x, y;
		Mat gray_frame;

                cvtColor(orig_frame, gray_frame, CV_BGR2GRAY);
                threshold(gray_frame, gray_frame, GRAY_THRESHOLD, 255, THRESH_BINARY);
		for(y = 0; y < 240; y += ROW_JUMPS)
			for(x = 0; x < 320; x += COLUMN_JUMPS)
				points.at<unsigned char>(y, x) = gray_frame.at<unsigned char>(y, x);
	}

#define POINTS_FOR_ANGLE 	10
	float get_angle(Mat &points, Mat &debug)
	{
		points.at<unsigned char>(y, x)
	}

	bool find_road_contour(Mat &orig_frame, double gray_threshold)
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

		Chicken::get_next_direction(navig, src, points, dir);
		cout << "!!!!RUDDER ANGLE=" << dir.get_angle()/M_PI*180 << endl;
                /* Correccion suave? */
                rudder_angle = dir.get_angle()/M_PI*180;
		rudder_value = rudder_angle/180;

                /* Correccion con giro */
             ///   if(abs(rudder_angle - 90) >= 40)
               //        navig->rotate(rudder_angle > 90? 30 : -30);
             //   else {
                       navig->forward(100, rudder_value);
	//	}

                return true;
	}

	bool align_to_line(Navig *navig, Mat &src, vector<Point> &points)
	{
		VLine dir;
		double rudder_angle;

		Chicken::get_next_direction(navig, src, points, dir);
		rudder_angle = dir.get_angle()/M_PI*180;
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

		do {
        		find_ret = Chicken::find_road_contour(src, points, thresholds[current_threshold], area);
			Chicken::get_next_direction(src, points, neg_line, pos_line, dir);
			angle_diff = (pos_line.get_angle() - neg_line.get_angle())/M_PI*180;
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
#if 0
					cout << "CALIBRATION RESULT: " << thresholds[current_threshold] << endl;
#endif
					need_calibration = false;
			}
		} while(need_calibration);

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
			return true;

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
