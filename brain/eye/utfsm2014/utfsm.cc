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
#include <algorithm>
#include "utfsm.h"
#include "debug.h"
#include <opencv2/imgproc/imgproc.hpp>

extern "C" {
#include <depth_sensor.h>
}

using namespace cv;
using namespace std;

static unsigned char color_min[3]; // = {99, 115, 114};
static unsigned char color_max[3]; //= {149, 145, 143};

void pieza_read_min_max()
{
		int v;
                ifstream config;
                int i;


                config.open("pieza.cfg", ifstream::in);
                for(i = 0; i < 3; i++) {
                        config >> v;
			color_min[i] = v;
#ifdef DEBUG
			cout << (unsigned int)color_min[i] << " ";
#endif
		}
		cout << endl;
                for(i = 0; i < 3; i++) {
                        config >> v;
			color_max[i] = v;
#ifdef DEBUG
                        cout <<	(unsigned int)color_max[i] << " ";
#endif
		}
		cout << endl;
                config.close();
}

static Mat frame;
bool filter_pieza(unsigned char *buf, vector<Point> &contour)
{
	int x, y;
	unsigned char *px;
	bool present = false;
	int count = 0;
	frame = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC3, (unsigned char *)buf);
	Mat gray = Mat(IMAGE_ROWS, IMAGE_COLUMNS, CV_8UC1);
	vector<vector<Point> > contours;
	unsigned char *buf2 = (unsigned char *)gray.data;

	for(y = 0; y < 240; y++)
		for(x = 0; x < 320; x++) {
			px = &buf[(320*y+x)*3];
			if(px[0] <= color_max[0] && px[1] <= color_max[1] && px[2] <= color_max[2] &&
			px[0] >= color_min[0] && px[1] >= color_min[1] && px[2] >= color_min[2]) {
				px = &buf2[(320*y+x)];
				px[0] = 255;
				px[1] = 255;
				px[2] = 255;
				count++;
				if(count > 17000)
					present = true;
			} else {
				px = &buf2[(320*y+x)];
				px[0] = 0;
				px[1] = 0;
				px[2] = 0;
			}
		}
	findContours(gray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	if(contours.size() > 0)
		contour = contours[0];
	cout << "COUNT: " << count << endl;
	return present;
}

static int get_angle(Point p1, Point p2)
{
        return (atan2(p2.y-p1.y, p2.x-p1.x)/CV_PI)*180;
}  

Object_HSV::Object_HSV(const Scalar &min, const Scalar &max)
{
	this->range[0][0] = min;
	this->range[0][1] = max;
	this->ranges_count = 1;
}

Object_HSV::Object_HSV(const Scalar &min0, const Scalar &max0, const Scalar &min1, const Scalar &max1)
{
        this->range[0][0] = min0;
        this->range[0][1] = max0;
	this->range[1][0] = min1;
	this->range[1][1] = max1;
        this->ranges_count = 2;
}

Object_HSV::Object_HSV(const Scalar &min0, const Scalar &max0, const Scalar &min1, const Scalar &max1, const Scalar &min2, const Scalar &max2)
{
        this->range[0][0] = min0;
        this->range[0][1] = max0;
        this->range[1][0] = min1;
        this->range[1][1] = max1;
	this->range[2][0] = min2;
        this->range[2][1] = max2;
        this->ranges_count = 3;
}

namespace Rescue {
	static const Object_HSV injured_color[3] = {
		//Object_HSV(Scalar(40, 76, 76), Scalar(75, 255, 255)),
		Object_HSV(Scalar(30, 30, 38), Scalar(75, 255, 255)),
		Object_HSV(Scalar(162, 51, 51), Scalar(179, 255, 255), Scalar(0, 51, 51), Scalar(18, 255, 255)),
		Object_HSV(Scalar(0, 0, 0), Scalar(180, 255, 40), Scalar(0, 0, 0), Scalar(20, 31, 69), Scalar(110, 0, 0), Scalar(180, 31, 69))
	};

	static const Object_HSV obstacle_color = Object_HSV(Scalar(22, 54, 94), Scalar(38, 255, 255));

	int avoid_obstacle_1(vector<Point> &points) 
	{
		Point left, right;
		double d_l, d_r;

                left = Rescue::point_find_nearest(points, Point(0, 240));
                right = Rescue::point_find_nearest(points, Point(320, 240));
                d_l = Rescue::calc_distance(left, Point(0, 240));
                d_r = Rescue::calc_distance(right, Point(320, 240));
#ifdef DEBUG
                cout << "bottom left distance: " << d_l << endl;
                cout << "bottom right distance:" << d_r << endl;
#endif
                if(d_r > d_l)
			return -1;
               	return 1;
	}

	bool detect_obstacle(vector<Point> &points)
	{
                Point left, right;
                double d_l, d_r;

                left = Rescue::point_find_nearest(points, Point(0, 240));
                right = Rescue::point_find_nearest(points, Point(320, 240));
                d_l = Rescue::calc_distance(left, Point(0, 240));
                d_r = Rescue::calc_distance(right, Point(320, 240));
#ifdef DEBUG
                cout << "bottom left distance: " << d_l << endl;
                cout << "bottom right distance:" << d_r << endl;
#endif
		if((d_r > 20.0 && right.y > 230) || (d_l > 20.0 && left.y > 230))
			return true;

                return false;
	}

        bool pass_start_line(vector<Point> &points)
        {
                Point left, right;

                left = Rescue::point_find_nearest(points, Point(0, 240));
                right = Rescue::point_find_nearest(points, Point(320, 240));
#ifdef DEBUG
		cout << "Right.y = " << right.y << endl;
		cout << "Left.y = " << left.y << endl;
#endif
                if(right.y > 230 || left.y > 230) {
                        return true;
		}

         	return false;
        }

	bool forward_road2(Mat &orig_frame, Navig *navig, short velocity, bool &pieza_present)
	{
		vector<vector<Point> > pieces;
		Rescue::road_find(orig_frame, pieces, pieza_present);
		if(pieces.size() > 0)
			return forward_road(orig_frame, navig, pieces, velocity, pieza_present);
		return true;
	}

	bool forward_road(Mat &orig_frame, Navig *navig, vector<vector<Point> > &pieces, short velocity, bool &pieza_present)
	{
		border_info binfo;
		unsigned int depth;
/*
		depth = depth_sensor_read();
		cout << "DEPTH SENSOR: " << depth << endl;
		if(depth >= 440) {
			navig->forward(velocity, 0.4);
		} else*/
		if(pieces.size() > 0 && pieces[0].size() > 3) {
			get_border_info(orig_frame, pieces, binfo);
			cout << "FORWARD: DISTANCE: " << binfo.distance << endl;
			if(binfo.distance <= 210) // +20*binfo.v_line_size/225.0)
				navig->forward(velocity, 0.7);
			else if(binfo.distance >= 240) // +20*binfo.v_line_size/225.0)
				navig->forward(velocity, 0.3);
			else
				navig->forward(velocity, 0.5);
			return true;
		} 
		return true;
	}

	double calc_distance(Point a, Point b)
	{
		return sqrt(pow((double)(b.x - a.x), 2.0) + pow((double)(b.y - a.y), 2.0));
	}

	Point point_find_nearest(vector<Point> &points, Point near_to, unsigned int &pos)
	{
		unsigned int i;

		double min_distance = calc_distance(points[0], near_to);
		double distance;
		pos = 0;
		for(i = 1; i < points.size(); i++) {
			distance = calc_distance(points[i], near_to);
			if(distance < min_distance) {
				min_distance = distance;
				pos = i;
			}
		}
		return points[pos];
	}

        Point point_find_nearest2(vector<Point> &points, Point near_to, unsigned int &pos)
        {
                unsigned int i;
		vector<Point> discarded;

                double min_distance = calc_distance(points[0], near_to);
                double distance;
                pos = 0;
                for(i = 1; i < points.size(); i++) {
			if(points[i].y >= 200) {
				discarded.push_back(points[i]);
				continue;
			}
                 	distance = calc_distance(points[i], near_to);
                        if(distance < min_distance) {
                                min_distance = distance;
                                pos = i;
                        }
                }
         	return points[pos];
        }

        Point point_find_nearest(vector<Point> &points, Point near_to)
        {
            	unsigned int pos;
                return point_find_nearest(points, near_to, pos);
        }

        struct contour_cmp {
                 bool operator()(vector<Point> const &a, vector<Point> const &b) const
                 {
                         return contourArea(a) > contourArea(b);
                 }              
        };

	void get_border_info(Mat &debug_frame, vector<vector<Point> > &contours, struct border_info &binfo)
	{
		double v_m;
		unsigned int p1_pos;
		unsigned int p2_pos;
		unsigned int p3_pos;
		Point p1 = point_find_nearest(contours[0], Point(30, 0), p1_pos);
		Point p2 = point_find_nearest2(contours[0], Point(0, 180), p2_pos);
		Point p3 = point_find_nearest(contours[0], Point(320, 0), p3_pos);
		Point p4;

		if(Debug::is_enabled()) {
			line(debug_frame, p1, p2, Scalar(0, 100, 150), 5);
			line(debug_frame, p1, p3, Scalar(100, 0, 150), 5);
		}
		binfo.v_angle = get_angle(p1, p2);
		binfo.h_angle = get_angle(p1, p3);
		binfo.v_line_size = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y-p1.y, 2));
		binfo.v_p1 = p1;
		binfo.v_p2 = p2;
		p3 = Point(170, 200);;
                v_m = (binfo.v_p2.y - binfo.v_p1.y)/((double)(binfo.v_p2.x - binfo.v_p1.x));
                p4.x = (int)(((p3.y-binfo.v_p1.y)/v_m)+binfo.v_p1.x);
                p4.y = p3.y;
                if(Debug::is_enabled()) {
                	line(debug_frame, p3, p4, Scalar(0, 100, 150), 5);
                }
                binfo.distance = calc_distance(p3, p4);
#if 0
		cout << "V BORDER ANGLE:" << binfo.v_angle << endl;
		cout << "H BORDER ANGLE:" << binfo.h_angle << endl;
		cout << "V SIZE:" << binfo.v_line_size << endl;
		cout << "DISTANCE:" << binfo.distance << endl;
#endif
	}

	
	void road_find(Mat &orig_frame, vector<vector<Point> > &contours_ret, bool &pieza_present)
	{
		Mat gray;
		unsigned int i;
		Mat road_mask;
		vector<vector<Point> > contours;
		vector<Point> pieza_contour;

		cvtColor(orig_frame, gray, CV_BGR2GRAY);
		inRange(gray, Scalar(100), Scalar(255), road_mask);
		findContours(road_mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

                if(filter_pieza((unsigned char *)orig_frame.data, pieza_contour)) {
			cout << "SIII" << endl;
                        contours.push_back(pieza_contour);
                        pieza_present = true;
                }

		sort(contours.begin(), contours.end(), contour_cmp());
		for(i = 0; i < 3 && i < contours.size(); i++) {
			approxPolyDP(contours[i], contours[i], 5.0, true);
#if 0
			cout << "count: " << contours[i].size() << endl;
			cout << "size: " << contourArea(contours[i]) << endl;
#endif
			if(contourArea(contours[i]) > 900)
				contours_ret.push_back(contours[i]);
		}

		if(Debug::is_enabled()) {
			//std::cout << "Contours: " << contours.size() << std::endl;
			Scalar colors[3] = {Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255)};
			for(i = 0; i < contours_ret.size(); i++) {
				drawContours(orig_frame, contours_ret, i, colors[i], CV_FILLED);
			}
		}
	}

	int box_find(Mat &hsv_frame, Mat &orig_frame, const Object_HSV &color, vector<Point> &points)
	{
		Mat box_mask[2];
		int cmax = 0;
		unsigned int i;
		double max_area;

#if 1
                std::cout << (int)hsv_frame.at<Vec3b>(120,160)[0] << " " << (int)hsv_frame.at<Vec3b>(120,160)[1] << " " << (int)hsv_frame.at<Vec3b>(120,160)[2] << std::endl;
#endif
		vector<vector<Point> > contours;

		inRange(hsv_frame, color.range[0][0], color.range[0][1], box_mask[0]);
		for(i = 1; i < color.ranges_count; i++) {
			inRange(hsv_frame, color.range[i][0], color.range[i][1], box_mask[1]);
			bitwise_or(box_mask[0], box_mask[1], box_mask[0]);
		}

		findContours(box_mask[0], contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		if(contours.size() == 0)
			return 0;

		max_area = contourArea(contours[0]);
		for(i = 1; i < contours.size(); i++) {
			double area = contourArea(contours[i]);
			if(area > max_area) {
				cmax = i;
				max_area = area;
			}
		}

		if(max_area < BOX_THRESHOLD)
			return 0;

		if(Debug::is_enabled())
			std::cout << "Points antes de aproximacion: " << contours[cmax].size() << std::endl;

		/* Rendimiento: En caso de obtener pocos frames, sacar esta aproximacion */
		approxPolyDP(contours[cmax], contours[cmax], 5.0, true);

		if(Debug::is_enabled()) {
			std::cout << "Points despues de aproximacion: " << contours[cmax].size() << std::endl;
			drawContours(orig_frame, contours, cmax, Scalar(0, 255, 0), CV_FILLED);
		}
		points = contours[cmax];
		return 1;
	}

	int injured_find(Mat &hsv_frame, Mat &orig_frame, unsigned int severity, vector<Point> &points)
	{
		if(severity > 2) {
			std::cout << "Atencion: severity=" << severity << " (Solo se permite de 0 a 2)" << std::endl;
			severity = 2;
		}
		return box_find(hsv_frame, orig_frame, injured_color[severity], points);
	}

	int obstacle_find(Mat &hsv_frame, Mat &orig_frame, vector<Point> &points)
	{
		return box_find(hsv_frame, orig_frame, obstacle_color, points);
	}

	void walls_find(Mat &hsv_frame, Mat &orig_frame)
	{
		Mat se11;
		Mat se21;
		Mat walls_mask;
		vector<vector<Point> > contours;

		inRange(hsv_frame, WALL_MIN, WALL_MAX, walls_mask);
		blur(walls_mask, walls_mask, Size(30,30));
		/*
		se11 = getStructuringElement(MORPH_RECT, Size(11, 11));
		se21 = getStructuringElement(MORPH_RECT, Size(21, 21));
		morphologyEx(step1, step2, MORPH_CLOSE, se21);
		morphologyEx(step2, walls_mask, MORPH_OPEN, se11);
		*/
		findContours(walls_mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		std::cout << "Contours: " << contours.size() << std::endl;
		std::cout << (int)hsv_frame.at<Vec3b>(120,160)[0] << " " << (int)hsv_frame.at<Vec3b>(120,160)[1] << " " << (int)hsv_frame.at<Vec3b>(120,160)[2] << std::endl;
		drawContours(orig_frame, contours, -1, Scalar(40, 100, 100), CV_FILLED);
	}
};
