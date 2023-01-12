#include "stdafx.h"
#include "CMissile.h"

CMissile::CMissile(cv::Size2f missile_size, cv::Point2f current_pos, cv::Point2f velocity){
	set_pos(current_pos);
	_velocity = velocity;
	_shape = cv::Rect2f(get_pos(), missile_size);
}