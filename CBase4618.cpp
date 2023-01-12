#include "stdafx.h"
#include "CBase4618.h"

#define DIGITAL 1

enum { RGBLED_BLU_PIN = 37, RGBLED_GRN_PIN, RGBLED_RED_PIN };

void CBase4618::run() {
	do {
		update();
		draw();

	} while (cv::waitKey(10) != 'q'); 

	//Turn off all LEDs
	port.set_data(DIGITAL, RGBLED_RED_PIN, 0);
	port.set_data(DIGITAL, RGBLED_GRN_PIN, 0);
	port.set_data(DIGITAL, RGBLED_BLU_PIN, 0);
}