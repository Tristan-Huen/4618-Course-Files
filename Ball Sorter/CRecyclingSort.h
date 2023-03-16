#pragma once
#include <CBase4618.h>
#include "cvui.h"
#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION

class CRecyclingSort : public CBase4618
{
    private:
        cv::Mat _frame;
        cv::Mat hsv_frame;
        cv::Mat _green_mask;
        cv::Mat _pink_mask;
        cv::Mat _blue_mask;
        cv::Mat _cropped;
        cv::Mat _erode;
        cv::Mat _ball_count_screen;
        cv::VideoCapture _vid;
        int _green_min;
        int _green_max;
        int _pink_min;
        int _pink_max;
        int _blue_min;
        int _blue_max;
        int _thresh_val;
        int _count_green_pixels;
        int _count_pink_pixels;
        int _count_blue_pixels;
        std::chrono::system_clock::time_point _start;
        bool _object_exists;
        bool _is_pressed_switch_one;
        bool _is_pressed_switch_two;
        int _system_state;
        int _bin_one_count;
        int _bin_two_count;
        int _bin_three_count;
    public:
        CRecyclingSort();
        void update();
        void draw();
        void get_button(int channel, bool &is_pressed);

};

