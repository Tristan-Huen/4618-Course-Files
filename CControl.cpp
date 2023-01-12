#include "stdafx.h"
#include "CControl.h"
#include <chrono>

#define HIGH 1
#define LOW 0

#define MAX_ADC_VAL 4096.0

enum { DIGITAL = 0, ANALOG, SERVO };

using namespace std;

void CControl::init_com(int comport) {
	string port = "COM" + to_string(comport);
	_com.open(port);
}

bool CControl::get_data(int type, int channel, int& result) {
	string rx_str;
	char buff[2];

	string message = "G " + to_string(type) + " " + to_string(channel) + "\n";
	_com.write(message.c_str(), message.length());

	//Code structure from original test_com() function in template file

	rx_str = "";
	// start timeout count
	double start_time = cv::getTickCount();

	buff[0] = 0;
	// Read 1 byte and if an End Of Line then exit loop
    // Timeout after 1 second, if debugging step by step this will cause you to exit the loop
	while (buff[0] != '\n' && (cv::getTickCount() - start_time) / cv::getTickFrequency() < 1.0)
	{
		if (_com.read(buff, 1) > 0)
		{
			rx_str = rx_str + buff[0];
		}
	}

	result = atoi(rx_str.substr(rx_str.find_last_of(" ") + 1, rx_str.length()).c_str());

}

bool CControl::set_data(int type, int channel, int value) {
	string rx_str;
	char buff[2];

	string set_str = "S " + to_string(type) + " " + to_string(channel) + " " + to_string(value) + "\n";

	_com.write(set_str.c_str(), set_str.length());

	//Code structure from original test_com() function in template file

	rx_str = "";
	// start timeout count
	double start_time = cv::getTickCount();

	buff[0] = 0;
	// Read 1 byte and if an End Of Line then exit loop
	// Timeout after 1 second, if debugging step by step this will cause you to exit the loop
	while (buff[0] != '\n' && (cv::getTickCount() - start_time) / cv::getTickFrequency() < 1.0)
	{
		if (_com.read(buff, 1) > 0)
		{
			rx_str = rx_str + buff[0];
		}
	}

}

void CControl::get_analog(int channel, float &result_percent, CControl &port) {
	int result;

	port.get_data(ANALOG, channel, result); 

	result_percent = result / MAX_ADC_VAL;
}