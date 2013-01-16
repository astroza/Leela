#include <iostream>
#include "utfsm12.h"
#include "debug.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

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
		float d_l, d_r;

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
                float d_l, d_r;

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

	bool avoid_obstacle_2(Navig *navig, vector<Point> &points, short velocity)
	{
		Point left, right;
		float d_l, d_r;
#ifdef DEBUG
        	cout << "Crab velocity: " << velocity << endl;
#endif
		navig->crab_move(velocity);
                if(velocity > 0) {
			left = Rescue::point_find_nearest(points, Point(0, 240));
                        d_l = Rescue::calc_distance(left, Point(0, 240));
#ifdef DEBUG
                        cout << "bottom left distance:" << d_l << endl;
#endif
                        if(d_l <= 10.0)
                        	return true;
		} else {
                	right = Rescue::point_find_nearest(points, Point(320, 240));
                        d_r = Rescue::calc_distance(right, Point(320, 240));
#ifdef DEBUG
                        cout << "bottom right distance:" << d_r << endl;
#endif
                        if(d_r <= 10.0)
                        	return true;
		}
		return false;
        }

	bool forward_road(Navig *navig, vector<Point> &points, short velocity)
	{
		Point left, right;
		float d_l, d_r;
                left = Rescue::point_find_nearest(points, Point(0, 0));
                right = Rescue::point_find_nearest(points, Point(320, 0));
#ifdef DEBUG
                cout << "Left: " << left.x << ", " << left.y << endl;
                cout << "Right: " << right.x << ", " << right.y << endl;
#endif
                d_l = Rescue::calc_distance(left, Point(0, 0));
                d_r = Rescue::calc_distance(right, Point(320, 0));
#ifdef DEBUG
                cout << "left distance: " << d_l << endl;
                cout << "right distance:" << d_r << endl;
                cout << "d_l / d_r = " << navig->to_rudder_value(d_l,  d_r) << endl;
#endif
                if(d_l != 0)
                	navig->forward(velocity, navig->to_rudder_value(d_l,  d_r));

                return true;
	}

	float calc_distance(Point a, Point b)
	{
		return sqrtf(powf((float)(b.x - a.x), 2.0) + powf((float)(b.y - a.y), 2.0));
	}

	Point point_find_nearest(vector<Point> &points, Point near_to)
	{
		int p = 0;
		unsigned int i;
		float min_distance = calc_distance(points[0], near_to);
		float distance;
		for(i = 1; i < points.size(); i++) {
			distance = calc_distance(points[i], near_to);
			if(distance < min_distance) {
				min_distance = distance;
				p = i;
			}
		}
		return points[p];
	}

	void road_find(Mat &hsv_frame, Mat &orig_frame, vector<Point> &points)
	{
		Mat gray;
		unsigned int i, cmax = 0;
		double max_area;

		cvtColor(orig_frame, gray, CV_BGR2GRAY);
		
		Mat road_mask;
		vector<vector<Point> > contours;
		inRange(gray, Scalar(100), Scalar(255), road_mask);
		findContours(road_mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);


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

		if(Debug::is_enabled()) {
			//std::cout << "Contours: " << contours.size() << std::endl;
			drawContours(orig_frame, contours, cmax, Scalar(0, 199, 224), CV_FILLED);
               	 	//std::cout << (int)hsv_frame.at<Vec3b>(120,160)[0] << " " << (int)hsv_frame.at<Vec3b>(120,160)[1] << " " << (int)hsv_frame.at<Vec3b>(120,160)[2] << std::endl;
		}
		points = contours[cmax];
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
