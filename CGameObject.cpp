#include "stdafx.h"
#include "CGameObject.h"

void CGameObject::move() {;
	set_pos(get_pos() + _velocity * 0.02);
	_shape = cv::Rect2f(get_pos(), _shape.size());
}

bool CGameObject::collide(CGameObject& obj) {

	return (_shape & obj._shape).area() > 0;
}

bool CGameObject::collide_wall(cv::Size board) {
	return (_shape.y >= board.height || _shape.y <= 0 || _shape.x >= board.width - _shape.width ||
		_shape.x <= 0);
}

void CGameObject::hit() {
	set_lives(get_lives() - 1);
}

void CGameObject::draw(cv::Mat& im) {
	cv::rectangle(im, _shape, cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
}