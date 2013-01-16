#ifndef AIE_H
#define AIE_H

#include <opencv2/imgproc/imgproc.hpp>
#include <navig.h>
#include "eye.h"

using namespace cv;
using namespace std;

class LineColor {
	public:
		Scalar range[2];
		LineColor(const Scalar &min, const Scalar &max);
};

class DRange {
	public:
		double min;
		double max;
		DRange(double min, double max);
		bool contains(double x);
};

class DRanges {
	private:
		vector<DRange> ranges;
	public:
		void add(const DRange &range);
		bool contains(double x);
};

class GLine {
	protected:
		Point A;
		Point B;
		bool used;
	private:
		Point r_A;
		Point r_B;
	public:
		GLine(const Point &r_A, const Point &r_B);
		Point get_A();
		Point get_B();
		virtual void digest(Point &p) = 0;
		void reset();
		double get_angle();
		double get_length();
		bool is_used();
		bool equals(GLine &l);
};

class VLine : public GLine
{
	public:
		void digest(Point &p);
		VLine();
};

class HLine : public GLine
{
	public:
		void digest(Point &p);
		HLine();
};

class Position
{
	private:
		Point current_point;
		int current_direction;
		static int directions[4][2];
	public:
		Position();
		int get_row() { return current_point.y; };
		int get_column() { return current_point.x; };
		int get_direction() { return current_direction; };
		void set_row(int row) { current_point.y = row%GRID_ROWS; };
		void set_column(int column) { current_point.x = column%GRID_COLUMNS; };
		void set_direction(int direction) { current_direction = direction < 0 ? 4 + direction%4 : direction%4; };
		Point get_point();
		int get_rotation_sign_for(int final_direction);
		int get_rotation_sign_for(Point dst_point);
		int find_direction_for(Point dst_point);
		void advance();
};

namespace Chicken {
	bool find_road_contour(Mat &orig_frame, vector<Point> &points, double gray_threshold, double &max_area);
	bool get_next_direction(Mat &orig_frame, vector<Point> &points, VLine &neg_line, VLine &pos_line, VLine &dir);
	bool detect_T(Mat &orig_frame, vector<Point> &points, double *distance);
	bool forward_Tless_line(Navig *navig, Mat &src);
	bool get_next_direction(Navig *navig, Mat &src, vector<Point> &points, VLine &dir);
	bool forward_line(Navig *navig, Mat &src, vector<Point> &points);
	bool forward_line(Navig *navig, Mat &src);
	bool align_to_line(Navig *navig, Mat &src, vector<Point> &points);
	bool align_to_line(Navig *navig, Mat &src);
	void init_dynamic_calibration(Eye *eye);
};

#endif
