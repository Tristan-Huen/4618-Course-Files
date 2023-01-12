#include "stdafx.h"
#include "CSketch.h"
#include <cmath>

#define DIGITAL 0
#define ANALOG 1

#define LOW 0
#define HIGH 1

#define DEBOUNCE_TIME 50

#define MAX_ADC_VAL 4096.0

#define JOY_Y_CHAN 4
#define JOY_X_CHAN 11

#define SW1 33
#define SW2 32

#define RED_COLOR cv::Scalar(0,0,255)
#define GRN_COLOR cv::Scalar(0,255,0)
#define BLU_COLOR cv::Scalar(255,0,0)

enum { RGBLED_BLU_PIN = 37, RGBLED_GRN_PIN, RGBLED_RED_PIN };

#define CANVAS_NAME "Etch-A-Sketch"

int CSketch::_color = 0;
bool CSketch::_reset = false;

CSketch::CSketch(int comport, cv::Size canvas_size) {
	port.init_com(comport);
	_canvas = cv::Mat::zeros(canvas_size, CV_8UC3);
}

void CSketch::get_analog() {

    //static values for only first time start up
    static int joy_x_val = -1;
    static int joy_y_val = -1;

    //Sleep for a bit to let analog values properly set in. Might just initial board startup effect
    if (joy_x_val == -1 && joy_y_val == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

	port.get_data(ANALOG, JOY_Y_CHAN, joy_y_val);
	port.get_data(ANALOG, JOY_X_CHAN, joy_x_val);

    _point = cv::Point(joy_x_val / MAX_ADC_VAL * _canvas.size().width, 
        (1.0 - joy_y_val / MAX_ADC_VAL) * _canvas.size().height);

}

void CSketch::update() {

    int led_colors[] = { RGBLED_RED_PIN, RGBLED_BLU_PIN, RGBLED_GRN_PIN };

	get_analog();

    get_button(SW1, &increment_color);
    get_button(SW2, true, &set_reset_state);

    //Move this to draw?
    port.set_data(DIGITAL, led_colors[get_color()], 1);

    //Turn off all other colors
    for (int color_index = 0; color_index < 3; color_index++) {
        if (color_index != get_color()) {
            port.set_data(DIGITAL, led_colors[color_index], 0);
        }
    }
}

void CSketch::draw() {
    cv::Scalar colors[] = { RED_COLOR, BLU_COLOR, GRN_COLOR };

    //Check if reset flag is set
    if (get_reset_state()) {
        set_reset_state(false);

        //Clear canvas
        _canvas = cv::Mat::zeros(cv::Size(_canvas.size().width, _canvas.size().height), CV_8UC3);
    }

    //Scale the coordinates to an ellipse double the size of the original containing the rectangular canvas
    _point.x = (_point.x - 0.5 * _canvas.size().width) * sqrt(2) + 0.5 * _canvas.size().width;
    _point.y = (_point.y - 0.5 * _canvas.size().height) * sqrt(2) + 0.5 * _canvas.size().height;

    //Following if statements are to restrict points to drawing in canvas. Must -1 to see a boundary line
    if (_point.x > _canvas.size().width) {
        _point.x = _canvas.size().width - 1;
    }

    if (_point.x < 0) {
        _point.x = 1;
    }

    if (_point.y > _canvas.size().height) {
        _point.y = _canvas.size().height-1;
    }

    if (_point.y < 0) {
        _point.y = 1;
    }

    //Need two points for a line so use last point to connect it to current one
	static cv::Point last_point = _point;

	cv::line(_canvas, _point, last_point, colors[get_color()], 1, cv::LINE_AA);

	cv::imshow(CANVAS_NAME, _canvas);

    //Store current point for next iteration
	last_point = _point;

}

void CSketch::get_button(int channel, static void (*function)()) {
    //Code structure based on https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
    //Chrono code from https://stackoverflow.com/questions/49785570/what-is-the-equivalent-of-millis-in-c/49785616

    static int button_state = 1;

    static int last_button_state = LOW;

    int reading;

    static long long last_debounce_time = 0;

    auto start = std::chrono::system_clock::now();

    port.get_data(DIGITAL, channel, reading);

    if (reading != last_button_state) {

        auto end = std::chrono::system_clock::now();
        last_debounce_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    auto end2 = std::chrono::system_clock::now();
    auto time_compare = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

    if ((time_compare - last_debounce_time) < DEBOUNCE_TIME) {
        if (reading != button_state) {
            button_state = reading;

            if (button_state == LOW) {
                function();
            }
        }
    }
}

void CSketch::get_button(int channel,bool flag_state, static void (*function)(bool)) {
    //Code structure based on https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
    //Chrono code from https://stackoverflow.com/questions/49785570/what-is-the-equivalent-of-millis-in-c/49785616

    static int button_state = 1;

    static int last_button_state = LOW;

    int reading;

    static long long last_debounce_time = 0;

    auto start = std::chrono::system_clock::now();

    port.get_data(DIGITAL, channel, reading);

    if (reading != last_button_state) {

        auto end = std::chrono::system_clock::now();
        last_debounce_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    auto end2 = std::chrono::system_clock::now();
    auto time_compare = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

    if ((time_compare - last_debounce_time) < DEBOUNCE_TIME) {
        if (reading != button_state) {
            button_state = reading;

            if (button_state == LOW) {
                function(flag_state);
            }
        }
    }
}

int CSketch::get_color() {
	return _color;
}

void CSketch::increment_color() {
	_color = (_color + 1) % 3;
}

bool CSketch::get_reset_state() {
    return _reset;
}

void CSketch::set_reset_state(bool flag_state) {
    _reset = flag_state;
}
