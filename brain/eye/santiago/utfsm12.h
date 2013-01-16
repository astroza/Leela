#ifndef UTFSM12_H
#define UTFSM12_H

#include <opencv2/core/core.hpp>
#include <navig.h>

#define BOX_THRESHOLD 7000.0

#define INJURED_0_MIN Scalar(40, 76, 76)
#define INJURED_0_MAX Scalar(75, 255, 255)
#define INJURED_1_MIN Scalar(0, 0, 0)
#define INJURED_1_MAX Scalar(0, 0, 0)

#define INJURED_2 Scalar(0, 0, 0)

#define WALL_MIN Scalar(100, 153, 54)
#define WALL_MAX Scalar(124, 255, 255) 

#define RUBBISH Scalar(0, 0, 0) 

#define OBSTACLE Scalar(0, 0, 0)

#define ROAD_MIN Scalar(0, 0, 120)
#define ROAD_MAX Scalar(360, 8, 255)

class Object_HSV {
	public:
		cv::Scalar range[2][2];
		unsigned int ranges_count;
		Object_HSV(const cv::Scalar &min, const cv::Scalar &max);
		Object_HSV(const cv::Scalar &min0, const cv::Scalar &max0, const cv::Scalar &min1, const cv::Scalar &max1);
		Object_HSV(const cv::Scalar &min0, const cv::Scalar &max0, const cv::Scalar &min1, const cv::Scalar &max1, const cv::Scalar &min2, const cv::Scalar &max2);
};

namespace Rescue {
        float calc_distance(cv::Point a, cv::Point b);
        cv::Point point_find_nearest(std::vector<cv::Point> &points, cv::Point near_to);
	void road_find(cv::Mat &hsv_frame, cv::Mat &orig_frame, std::vector<cv::Point> &points);
        void walls_find(cv::Mat &hsv_frame, cv::Mat &orig_frame);
	int box_find(cv::Mat &hsv_frame, cv::Mat &orig_frame, const Object_HSV &color, std::vector<cv::Point> &points);
	int injured_find(cv::Mat &hsv_frame, cv::Mat &orig_frame, unsigned int severity, std::vector<cv::Point> &points);
	int obstacle_find(cv::Mat &hsv_frame, cv::Mat &orig_frame, std::vector<cv::Point> &points);

	int avoid_obstacle_1(std::vector<cv::Point> &points);
	bool avoid_obstacle_2(Navig *navig, std::vector<cv::Point> &points, short velocity);
	bool forward_road(Navig *navig, std::vector<cv::Point> &points, short velocity);
	bool detect_obstacle(std::vector<cv::Point> &points);
	bool pass_start_line(std::vector<cv::Point> &points);
};

#endif
