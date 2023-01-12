#include "stdafx.h"
#include "CSpecialInvader.h"

#define SCALE_FACTOR 3
#define PIX_WIDTH 16
#define PIX_HEIGHT 7

int CSpecialInvader::sprite[] = {
	0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0, //.....@@@@@@.....
	0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0, //...@@@@@@@@@@...
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0, //..@@@@@@@@@@@@..
	0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0, //.@@.@@.@@.@@.@@.
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //@@@@@@@@@@@@@@@@
	0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0, //..@@@..@@..@@@..
	0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0  //...@........@...
};

CSpecialInvader::CSpecialInvader() {

}


CSpecialInvader::CSpecialInvader(cv::Point2f initial_pos) {
	_position = initial_pos;
	set_lives(0);

	_shape = cv::Rect2f(_position, cv::Size2f(PIX_WIDTH * SCALE_FACTOR, PIX_HEIGHT * SCALE_FACTOR));
}

void CSpecialInvader::draw(cv::Mat &im) {
	for (int col = 0; col < PIX_HEIGHT; col++) {
		for (int rows = 0; rows < PIX_WIDTH; rows++) {
			cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
				_position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
				sprite[(rows + PIX_WIDTH * col)] * cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_AA);
		}
	}
}