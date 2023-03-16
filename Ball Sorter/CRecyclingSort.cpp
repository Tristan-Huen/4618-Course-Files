#include "CRecyclingSort.h"
#include <thread>
#define DEBOUNCE_TIME 10
#define GATE_CLOSE_TIME 173 //182 on outlet power, 160 or 161 on USB power

/*
NOTES:
-Takes 1.12s approximately for the ball to roll into one of the bins
-Bin1 is 1500ms position
-Bin 2 is 2200ms positon
*/

CRecyclingSort::CRecyclingSort() {
    port.init_com(3);

    _green_min = 49;
    _green_max = 80;
    _pink_min = 146;
    _pink_max = 170;
    _blue_min = 90;
    _blue_max = 105;
    _thresh_val = 35;
    _system_state = -1;
    _bin_one_count = 0;
    _bin_two_count = 0;
    _bin_three_count = 0;
    _is_pressed_switch_one = false;
    _is_pressed_switch_two = false;
    _vid.open(0);

    //Fixes lag issues (thanks Anton). Note this must be a 4:3 aspect ratio or else errors occur
    _vid.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    _vid.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cvui::init("Original");

    _start = std::chrono::system_clock::now();

}

void CRecyclingSort::update() {
    _vid >> _frame;

    bool is_pressed = false;

    cv::Mat gray_scale, thresh, hsv_frame;

    get_button(3,is_pressed);

    if(is_pressed) {
        _system_state *= -1;
    }

    if(_vid.isOpened()) {
        _object_exists = false;
        _cropped = _frame(cv::Rect(cv::Point(_frame.size().width * 0.5 - 100, _frame.size().height * 0.5 - 100),
            cv::Size(200, 200)));

         cv::cvtColor(_cropped, hsv_frame, cv::COLOR_BGR2HSV);
         cv::cvtColor(_cropped, gray_scale, cv::COLOR_BGR2GRAY);

         cv::threshold(gray_scale, thresh, _thresh_val, 70, cv::THRESH_BINARY);

         cv::erode(thresh, _erode, cv::Mat(),cv::Point(-1,-1),11);

         std::vector<cv::Vec4i> hierarchy;
         std::vector<std::vector<cv::Point>> contours;
         std::vector<cv::Point> contour;
         findContours(_erode, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
         for (unsigned int i = 0; i < contours.size(); i++) {

             if (cv::contourArea(contours[i]) >= 3000) {
                 drawContours(_cropped, contours, i, cv::Scalar(0, 255, 0), cv::LINE_AA, 8, hierarchy);
                 _object_exists = true;
             }

         }

         cv::Scalar min_green = cv::Scalar(_green_min, 25, 25);
         cv::Scalar max_green = cv::Scalar(_green_max, 255, 255);

         cv::Scalar min_pink = cv::Scalar(_pink_min, 67, 67);
         cv::Scalar max_pink = cv::Scalar(_pink_max, 255, 255);

         cv::Scalar min_blue = cv::Scalar(_blue_min, 67, 67);
         cv::Scalar max_blue = cv::Scalar(_blue_max, 255, 255);

         cv::inRange(hsv_frame, min_green, max_green, _green_mask);
         cv::inRange(hsv_frame, min_pink, max_pink, _pink_mask);
         cv::inRange(hsv_frame, min_blue, max_blue, _blue_mask);

         _count_green_pixels = cv::countNonZero(_green_mask);
         _count_pink_pixels = cv::countNonZero(_pink_mask);
         _count_blue_pixels = cv::countNonZero(_blue_mask);

         auto end = std::chrono::system_clock::now();

         if((std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count() >= 1500) && _object_exists && (_system_state == 1)) {
             _start = std::chrono::system_clock::now();
             port.set_data(SERVO, 2, 800);

             if(_count_green_pixels >= 1000) {
                 port.set_data(SERVO, 4, 1500);
                 port.set_data(DIGITAL, 19, 1);
                 _bin_one_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
             }

             else if (_count_pink_pixels >= 1000) {
                 port.set_data(SERVO, 4, 2200);
                 port.set_data(SERVO, 17, 800);
                 _bin_three_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
                 port.set_data(SERVO, 2, 1700);
                 std::this_thread::sleep_for(std::chrono::milliseconds(1300));
                 port.set_data(SERVO, 17, 1500);
             }

             else if (_count_blue_pixels >= 1000) {
                 port.set_data(SERVO, 4, 1500);
                 port.set_data(SERVO, 10, 1100);
                 _bin_three_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
                 port.set_data(SERVO, 2, 1700);
                 std::this_thread::sleep_for(std::chrono::milliseconds(1300));
                 port.set_data(SERVO, 10, 660);
             }

             else {
                 port.set_data(SERVO, 4, 2200);
                 port.set_data(DIGITAL, 13, 1);
                 _bin_two_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
            }

         }

         else if( _system_state == -1) {

             get_button(27, _is_pressed_switch_one);
             get_button(22, _is_pressed_switch_two);

             port.set_data(SERVO, 2, 800);

             if(_is_pressed_switch_one && _object_exists) {
                 _is_pressed_switch_one = false;
                 port.set_data(SERVO, 4, 1500);
                 port.set_data(DIGITAL, 19, 1);
                 _bin_one_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
             }

             else if (_is_pressed_switch_two && _object_exists) {
                 _is_pressed_switch_two = false;
                 port.set_data(SERVO, 4, 2200);
                 port.set_data(DIGITAL, 13, 1);
                 _bin_two_count++;
                 std::this_thread::sleep_for(std::chrono::milliseconds(GATE_CLOSE_TIME));
             }


         }

         port.set_data(SERVO, 2, 1700); //800 is on the right
         port.set_data(DIGITAL, 19, 0);
         port.set_data(DIGITAL, 13, 0);
    }

}

void CRecyclingSort::draw() {

    _ball_count_screen = cv::Mat::zeros(cv::Size(500,500), CV_8UC3);

    if (_count_green_pixels >= 1000 && _object_exists) {
        cv::putText(_frame, "Green Ball: BIN1",
        cv::Point(_frame.size().width / 4 - 120, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(0, 255, 0), 1, cv::LINE_AA);

    }

    else if (_count_pink_pixels >= 1000 && _object_exists) {
        cv::putText(_frame, "Pink Ball: BIN3",
        cv::Point(_frame.size().width / 4 - 120, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(139, 57, 236), 1, cv::LINE_AA);

    }

    else if (_count_blue_pixels >= 1000 && _object_exists) {
        cv::putText(_frame, "Blue Ball: BIN4",
        cv::Point(_frame.size().width / 4 - 120, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 0, 0), 1, cv::LINE_AA);

    }

    else if (_object_exists) {
        cv::putText(_frame, "Ball: BIN2",
            cv::Point(_frame.size().width / 4 - 120, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    }

    else {
        cv::putText(_frame, "NO OBJECT",
           cv::Point(_frame.size().width / 4 - 120, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
           cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    }

    if(_system_state == 1) {
        port.set_data(DIGITAL, 26, 1);
        cv::putText(_frame, "SYSTEM ONLINE",
        cv::Point(_frame.size().width / 4 + 170, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    }

    else {
        port.set_data(DIGITAL, 26, 0);
        cv::putText(_frame, "SYSTEM OFFLINE",
        cv::Point(_frame.size().width / 4 + 170, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }

    cv::putText(_ball_count_screen, "BIN 1 Count: " + std::to_string(_bin_one_count),
        cv::Point(50,50), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(0, 255, 0), 1, cv::LINE_AA);

    cv::putText(_ball_count_screen, "BIN 2 Count: " + std::to_string(_bin_two_count),
        cv::Point(50,80), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

    cv::putText(_ball_count_screen, "BIN 3 Count: " + std::to_string(_bin_three_count),
        cv::Point(50,110), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(139, 57, 236), 1, cv::LINE_AA);

    cvui::trackbar(_frame, 40, 30, 220, &_blue_min, 80, 110);
    cvui::trackbar(_frame, 40, 90, 220, &_blue_max, 80, 110);
    cvui::trackbar(_frame, 40, 150, 220, &_thresh_val, 10, 70);

    if(cvui::button(_frame, 40, 220, 100, 30, "ON/OFF")) {
        _system_state *= -1;
    }


    if(cvui::button(_frame, 40, 290, 100, 30, "BIN1") && (_system_state == -1)) {
        _is_pressed_switch_one = true;
    }

    if(cvui::button(_frame, 40, 360, 100, 30, "BIN2") && (_system_state == -1)) {
        _is_pressed_switch_two = true;
    }

    if(cvui::button(_frame, 180, 220, 100, 30, "CLEAR")) {
        _bin_one_count = 0;
        _bin_two_count = 0;
        _bin_three_count = 0;
    }

    cvui::update();

    cv::imshow("Green Mask", _green_mask);
    cv::imshow("Pink Mask", _pink_mask);
    cv::imshow("Blue Mask", _blue_mask);
    cv::imshow("Ball Count", _ball_count_screen);
    cv::imshow("Cropped Original", _cropped);
    cv::imshow("Original", _frame);

}

void CRecyclingSort::get_button(int channel, bool &is_pressed) {
    static int button_state = 1;
    static int last_button_state = 0;

    int reading;

    static long long last_debounce_time = 0;

    auto start = std::chrono::system_clock::now();

    port.get_data(DIGITAL,channel, reading);

    if (reading != last_button_state) {
        auto end = std::chrono::system_clock::now();
        last_debounce_time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    }

    auto end2 = std::chrono::system_clock::now();
    auto time_compare = std::chrono::duration_cast<std::chrono::milliseconds>(end2-start).count();

    if((time_compare - last_debounce_time) < DEBOUNCE_TIME) {
        if(reading != button_state) {
            button_state = reading;

            if(button_state == 0) {
                is_pressed = true;
            }

            else {
                is_pressed = false;
            }
        }
    }
}



