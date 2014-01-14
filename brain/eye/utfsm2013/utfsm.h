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

#ifndef UTFSM12_H
#define UTFSM12_H

#include <opencv2/core/core.hpp>
#include <navig.h>

#define BOX_THRESHOLD 7000.0

#define WALL_MIN Scalar(100, 153, 54)
#define WALL_MAX Scalar(124, 255, 255) 

#define RUBBISH Scalar(0, 0, 0) 

#define OBSTACLE Scalar(0, 0, 0)

#define ROAD_MIN Scalar(0, 0, 120)
#define ROAD_MAX Scalar(360, 8, 255)

struct border_info {
	cv::Point v_p1;
	cv::Point v_p2;
	double v_line_size;
	double v_angle;
	double h_angle;
	double distance;
};

class Object_HSV {
	public:
		cv::Scalar range[2][2];
		unsigned int ranges_count;
		Object_HSV(const cv::Scalar &min, const cv::Scalar &max);
		Object_HSV(const cv::Scalar &min0, const cv::Scalar &max0, const cv::Scalar &min1, const cv::Scalar &max1);
		Object_HSV(const cv::Scalar &min0, const cv::Scalar &max0, const cv::Scalar &min1, const cv::Scalar &max1, const cv::Scalar &min2, const cv::Scalar &max2);
};

namespace Rescue {
        double calc_distance(cv::Point a, cv::Point b);
        cv::Point point_find_nearest(std::vector<cv::Point> &points, cv::Point near_to);
	cv::Point point_find_nearest(std::vector<cv::Point> &points, cv::Point near_to, unsigned int &pos);
	cv::Point point_find_nearest2(std::vector<cv::Point> &points, cv::Point near_to, unsigned int &pos);
	void road_find(cv::Mat &orig_frame, std::vector<std::vector<cv::Point> > &contours_ret, bool &);
	int box_find(cv::Mat &hsv_frame, cv::Mat &orig_frame, const Object_HSV &color, std::vector<cv::Point> &points);
	void get_border_info(cv::Mat &debug_frame, std::vector<std::vector<cv::Point> > &contours, struct border_info &binfo);
	void get_border_info(cv::Mat &debug_frame, std::vector<std::vector<cv::Point> > &contours, struct border_info &binfo);
bool forward_road(cv::Mat &orig_frame, Navig *navig, std::vector<std::vector<cv::Point> > &pieces, short velocity, bool &pieza_present);
	bool forward_road2(cv::Mat &orig_frame, Navig *navig, short velocity, bool &pieza_present); 
	bool detect_obstacle(std::vector<cv::Point> &points);
	bool pass_start_line(std::vector<cv::Point> &points);
};

void pieza_read_min_max();
bool filter_pieza(unsigned char *buf, std::vector<cv::Point> &);
#endif
