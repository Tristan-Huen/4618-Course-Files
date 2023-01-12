#include "stdafx.h"
#include "CShip.h"

CShip::CShip() {

}

CShip::CShip(cv::Size2f ship_size, int lives, cv::Point2f velocity) {
	_width = ship_size.width;
	_height = ship_size.height;
	_velocity = velocity;
	_position = cv::Point2f(500,700-25);
	_shape = cv::Rect2f(get_pos(), ship_size);
	set_lives(lives);
}

void CShip::draw(cv::Mat& im) {
	//Color for scalar is B G R
	cv::rectangle(im, _shape, cv::Scalar(0, 255, 51), cv::FILLED, cv::LINE_AA);
}