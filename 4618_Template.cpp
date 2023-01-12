////////////////////////////////////////////////////////////////
// ELEX 4618 Template project for BCIT
// Created Oct 5, 2016 by Craig Hennessey
// Last updated Dec 6, 2021
////////////////////////////////////////////////////////////////
#include "stdafx.h"

// Add simple GUI elements
#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION
#include "cvui.h"

#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cmath>
#include <vector>

#include "Client.h"
#include "Server.h"

// Must include Windows.h after Winsock2.h, so Serial must be included after Client/Server
#include "Serial.h" 

#include "CControl.h"
#include "CSketch.h"
#include "CPong.h"
#include "CSpaceInvaderGame.h"

#define CANVAS_NAME "Display Image"

#define HIGH 1
#define LOW 0

#define DEBOUNCE_TIME 1000

#define COM3 3

#define MAX_SERVO_ANGLE 178
#define MIN_SERVO_ANGLE 1

#define SW1 33

#define JOY_Y_CHAN 4
#define JOY_X_CHAN 11

#define MAX_ADC_VAL 4096
#define BALL_RADIUS 10.0

#define SCALE_FACTOR 3
#define INVADERS_COL_NUM 11
#define INVADERS_ROW_NUM 5

enum { DIGITAL = 0, ANALOG, SERVO };

enum { RGBLED_BLU_PIN = 37, RGBLED_GRN_PIN, RGBLED_RED_PIN };

void print_menu()
{
	std::cout << "\n***********************************";
	std::cout << "\n* ELEX4618 Template Project";
	std::cout << "\n***********************************";
    std::cout << "\n(1) Test Joystick";
    std::cout << "\n(2) Test Digital I/O";
    std::cout << "\n(3) Test Debounced Button";
    std::cout << "\n(4) Test Servo Rotation";
    std::cout << "\n(5) Etch-A-Sketch";
    std::cout << "\n(6) Pong";
    std::cout << "\n(7) Space Invaders";
    std::cout << "\n(8) Test Sprite";
    std::cout << "\n(9) Do Video";
    std::cout << "\n(10) Test Server";
	std::cout << "\n(0) Exit";
	std::cout << "\nCMD> ";
}

void do_video()
{
    cv::VideoCapture vid;

    vid.open(0);

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

    bool do_canny = true;
    bool do_aruco = false;
    int canny_thresh = 30;
    cvui::init(CANVAS_NAME);
    cv::Point set_pt = cv::Point(10, 50);
    std::vector<cv::Scalar> color_vec;
    color_vec.push_back(cv::Scalar(255, 255, 255));
    color_vec.push_back(cv::Scalar(255, 0, 0));
    color_vec.push_back(cv::Scalar(0, 255, 0));
    color_vec.push_back(cv::Scalar(0, 0, 255));
    int color_index = 0;
    if (vid.isOpened() == TRUE)
    {
        do
        {
            cv::Mat frame, edges;
            vid >> frame;

            if (frame.empty() == false)
            {
                if (do_aruco == true)
                {
                    std::vector<int> ids;
                    std::vector<std::vector<cv::Point2f> > corners;
                    cv::aruco::detectMarkers(frame, dictionary, corners, ids);
                    if (ids.size() > 0)
                    {
                        cv::aruco::drawDetectedMarkers(frame, corners, ids);
                    }
                }

                if (do_canny == true)
                {
                    cv::cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
                    cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
                    cv::Canny(edges, edges, 0, canny_thresh, 3);
                    cv::add(frame, color_vec.at(color_index), frame, edges);
                }

                cvui::window(frame, set_pt.x, set_pt.y, 200, 190, "Settings");
                cvui::checkbox(frame, set_pt.x + 5, set_pt.y + 25, "Canny Filter", &do_canny);
                cvui::checkbox(frame, set_pt.x + 5, set_pt.y + 50, "ArUco", &do_aruco);
                cvui::text(frame, set_pt.x + 5, set_pt.y + 75, "Canny Threshold");
                cvui::trackbar(frame, set_pt.x + 5, set_pt.y + 90, 180, &canny_thresh, 5, 120);
                if (cvui::button(frame, set_pt.x + 5, set_pt.y + 140, 100, 30, "Colour Switch"))
                {
                    color_index++;
                    if (color_index >= color_vec.size()) { color_index = 0; }
                }

                cvui::update();
                cv::imshow(CANVAS_NAME, frame);
            }
        } while (cv::waitKey(10) != ' ');
    }
}

void test_joystick() {
    CControl port;

    port.init_com(3);

    //port.init_com(3);

    float joy_x_percent = 0;
    float joy_y_percent = 0;

    std::cout.precision(3);

    do {

        //Possibly use this keyword?
        port.get_analog(JOY_X_CHAN, joy_x_percent, port);
        port.get_analog(JOY_Y_CHAN, joy_y_percent, port);

        std::cout << "ANALOG TEST: CH11 X = " << (int)(joy_x_percent * MAX_ADC_VAL) << "(" << joy_x_percent * 100.0 
            << "%) CH4 Y = " << (int)(joy_y_percent * MAX_ADC_VAL)  << "(" << joy_y_percent * 100.0 << "%)\n";

        cv::waitKey(1);

    } while (1);

}

void toggle_led() {
    CControl port;

    port.init_com(COM3);

    int button_state = LOW;

    do {

        port.get_data(DIGITAL, 33, button_state); 

        if (button_state == LOW) {
            port.set_data(DIGITAL, RGBLED_GRN_PIN, HIGH);
            std::cout << "DIGITAL TEST: CH38 = " << HIGH << "\n";
        }

        else {
            port.set_data(DIGITAL, RGBLED_GRN_PIN, LOW);
            std::cout << "DIGITAL TEST: CH38 = " << LOW << "\n";
        }

    } while (1);
}


void get_button() {

    CControl port;

    port.init_com(COM3);

    int button_state = 1;

    int last_button_state = LOW;

    int reading;

    int button_count = 0;

    long long last_debounce_time = 0; 

    //Code structure based on https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
    //Chrono code from https://stackoverflow.com/questions/49785570/what-is-the-equivalent-of-millis-in-c/49785616

    do {

        auto start = std::chrono::system_clock::now();

        port.get_data(DIGITAL, SW1, reading);

        if (reading != last_button_state) {

            auto end = std::chrono::system_clock::now();
            last_debounce_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        auto end2 = std::chrono::system_clock::now();
        auto time_compare = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start).count();

        //Change to 1000 ms timeout temporarily
        if ((time_compare - last_debounce_time) < DEBOUNCE_TIME) {
            if (reading != button_state) {
                button_state = reading;

                if (button_state == LOW) {
                    button_count++;
                    std::cout << "BUTTON TEST: " << button_count << "\n";
                }
            }
        }

    } while (1);

}

void rotate_servo() {
    CControl port;

    port.init_com(COM3);

    //Set initial servo starting position to angle of 1 degree
    port.set_data(SERVO, 0, MIN_SERVO_ANGLE);

    do {

        //Rotate from min angle to max angle
        for (int angle = MIN_SERVO_ANGLE; angle <= MAX_SERVO_ANGLE; angle++) {
            std::cout << "SERVO TEST CH: " << angle << "\n";
            port.set_data(SERVO, 0, angle);
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Slow down servo rotate speed
        }

        //Rotate from max angle to min angle
        for (int angle = MAX_SERVO_ANGLE; angle >= MIN_SERVO_ANGLE; angle--) {
            std::cout << "SERVO TEST CH: " << angle << "\n";
            port.set_data(SERVO, 0, angle);
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Slow down servo rotate speed
        }

    } while (1);
}

void etch_sketch() {
    CSketch sketch = CSketch();
    sketch.run();

}

void pong() {
    CPong pong = CPong();
    pong.run();
}

void spaceInvaders() {
    CSpaceInvaderGame game = CSpaceInvaderGame();
    game.run();
}

class Invader {
private:
    cv::Rect2f _hitbox;
    cv::Point _velocity = 300;
    cv::Point _position;
    const int _sprite_height = 8;
    const int _sprite_main_width = 11;
    const int _sprite_tiny_width = 8;
    const int _sprite_big_width = 12;

    int sprite_tiny_squid[88] = {
        0,0,0,1,1,0,0,0,          // ...@@...
        0,0,1,1,1,1,0,0,          // ..@@@@..
        0,1,1,1,1,1,1,0,          // .@@@@@@.
        1,1,0,1,1,0,1,1,          // @@.@@.@@
        1,1,1,1,1,1,1,1,          // @@@@@@@@
        0,0,1,0,0,1,0,0,          // ..@..@.. 
        0,1,0,0,0,0,1,0,          // .@....@.
        0,0,1,0,0,1,0,0           // ..@..@..
    };

    int sprite_tiny_squid_two[88] = {
        0,0,0,1,1,0,0,0,          // ...@@...
        0,0,1,1,1,1,0,0,          // ..@@@@..
        0,1,1,1,1,1,1,0,          // .@@@@@@.
        1,1,0,1,1,0,1,1,          // @@.@@.@@
        1,1,1,1,1,1,1,1,          // @@@@@@@@
        0,0,1,0,0,1,0,0,          // ..@..@.. 
        0,1,0,1,1,0,1,0,          // .@.@@.@.
        1,0,1,0,0,1,0,1           // @.@..@.@
    };

    int sprite_big_squid[96]= {
        0,0,0,0,1,1,1,1,0,0,0,0,  // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0,  // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1,  // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        0,0,0,1,1,0,0,1,1,0,0,0,  // ...@@..@@...
        0,0,1,0,0,1,1,0,0,1,0,0,  // ..@..@@..@..
        0,0,0,1,0,0,0,0,1,0,0,0   // ...@....@...
    };

    int sprite_big_squid_two[96] = { //96
        0,0,0,0,1,1,1,1,0,0,0,0,  // ....@@@@.... 
        0,1,1,1,1,1,1,1,1,1,1,0,  // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1,  // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        0,0,0,1,1,0,0,1,1,0,0,0,  // ...@@..@@...
        0,0,1,1,0,1,1,0,1,1,0,0,  // ..@@.@@.@@..
        1,1,0,0,0,0,0,0,0,0,1,1   // @@........@@
    };

    int sprite_main[88] = {
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
    0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
    0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
    0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
    };

    int sprite_main_two[88] = {
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
    };
public:
    Invader(cv::Point initial_pos, int sprite_choice);
    void draw(int animation, int sprite_choice, cv::Mat& im);
    void move();

    cv::Point get_velocity() { return _velocity; }
    void set_velocity(cv::Point velocity) { _velocity = velocity; }

    cv::Point get_pos() { return _position; }
    void set_pos(cv::Point position) { _position = position; }

    int get_width() { return _hitbox.width; }
    int get_height() { return _hitbox.height; }
};

Invader::Invader(cv::Point initial_pos, int sprite_choice) {
    _position = initial_pos;
    switch (sprite_choice) {
    case 1:
        _hitbox = cv::Rect2f(_position, cv::Size2f(_sprite_main_width * SCALE_FACTOR, _sprite_height * SCALE_FACTOR));
        break;
    case 2:
        _hitbox = cv::Rect2f(_position, cv::Size2f(_sprite_tiny_width * SCALE_FACTOR, _sprite_height * SCALE_FACTOR));
    case 3:
        _hitbox = cv::Rect2f(_position, cv::Size2f(_sprite_big_width * SCALE_FACTOR, _sprite_height * SCALE_FACTOR));
    }   
}

void Invader::draw(int animation, int sprite_choice, cv::Mat& im) {

    if (sprite_choice == 1) {
        if (animation == 1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_main_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_main[(rows + 11 * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == -1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_main_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_main_two[(rows + 11 * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }
    }

    else if (sprite_choice == 2) {
        if (animation == 1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_tiny_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_tiny_squid[(rows + _sprite_tiny_width * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == -1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_tiny_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_tiny_squid_two[(rows + _sprite_tiny_width * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }
        
    }

    else if (sprite_choice == 3) {
        if (animation == 1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_big_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_big_squid[(rows + _sprite_big_width * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == -1) {
            for (int col = 0; col < _sprite_height; col++) {
                for (int rows = 0; rows < _sprite_big_width; rows++) {
                    cv::rectangle(im, _position + cv::Point(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_big_squid_two[(rows + _sprite_big_width * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }
    }

    
}

void Invader::move() {
    _position += _velocity * 0.01;
}

void test_sprite() {
    cv::Mat canvas = cv::Mat::zeros(cv::Size(1000, 700), CV_8UC3);
    std::vector<Invader> _invaders; ///< Vector of invaders

    for (int row_count = 0; row_count < INVADERS_ROW_NUM; row_count++) {
        for (int invader_index = 0; invader_index < INVADERS_COL_NUM; invader_index++) {
            Invader invader = Invader(cv::Point2f(180 + invader_index * 60, 700 - 610 + row_count * 50),3);
            invader.set_velocity(cv::Point2f(300, 0));
            _invaders.push_back(invader);
        }
    }

    auto start = std::chrono::system_clock::now();
    int animation = 1;

    do {
        canvas = cv::Mat::zeros(canvas.size(), CV_8UC3);

        auto end = std::chrono::system_clock::now();

        //If enough frame time has elapsed then move the invaders
        if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() >= 200) {
            start = std::chrono::system_clock::now();
            animation *= -1;


            //Move invaders
            for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                if (_invaders[invader_index].get_pos().x + _invaders[invader_index].get_width() >= canvas.size().width) {
                    for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                        _invaders[invader_index].set_velocity(cv::Point(0, 300));
                        _invaders[invader_index].move();
                        _invaders[invader_index].set_velocity(cv::Point(-300, 0));
                    }
                    break;
                }

                if (_invaders[invader_index].get_pos().x <= 0) {
                    for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                        _invaders[invader_index].set_velocity(cv::Point(0, 300));
                        _invaders[invader_index].move();
                        _invaders[invader_index].set_velocity(cv::Point(300, 0));
                    }
                    break;
                }
                
            }
            for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                _invaders[invader_index].move();
            }

        }

        auto time_a = std::chrono::system_clock::now();

        //Draw all invaders
        /*for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
            _invaders[invader_index].draw(animation,3, canvas);
        }*/

        for (auto invaders : _invaders) {
            invaders.draw(animation, 3, canvas);
        }

        auto time_b = std::chrono::system_clock::now();

        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(time_b - time_a).count() << "\n";

        cv::imshow("Sprite Test", canvas);
    } while (cv::waitKey(10) != 'q');
}

void process_msg()
{
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

////////////////////////////////////////////////////////////////
// Demo client server communication
////////////////////////////////////////////////////////////////
bool serverthreadexit = false;
Server serv;

// Send image to TCP server
void serverimagethread()
{
    cv::VideoCapture vid;

    vid.open(0);

    if (vid.isOpened() == true)
    {
        do
        {
            cv::Mat frame;
            vid >> frame;
            if (frame.empty() == false)
            {
                imshow("Server Image", frame);
                process_msg();
                serv.set_txim(frame);
            }
        } while (serverthreadexit == false);
    }
}

void serverthread()
{
    // Start server
    serv.start(4618);
}

void server()
{
    char inputchar;
    bool video_live = false;
    std::vector<std::string> cmds;

    /*cv::VideoCapture vid;

    vid.open(0);*/

    std::thread t1(&serverimagethread);
    t1.detach();

    // Start server thread
    std::thread t2(&serverthread);
    t2.detach();

    cv::namedWindow("WindowForWaitkey");
    do
    {
        inputchar = cv::waitKey(100);
        if (inputchar == 'q')
        {
            serverthreadexit = true;
        }

        serv.get_cmd(cmds);

        if (cmds.size() > 0) {
        
            for (int i = 0; i < cmds.size(); i++)
            {
                //Set commands
                if (cmds[i][0] == 'S') {

                    //System commands
                    if (cmds[i][2] == '0') {
                        if (cmds[i][4] == '1') {

                            std::cout << "\nSystem is switched on";

                            std::string reply = "Recieved S 0 1 Command";
                            serv.send_string(reply);

                        }

                        else if (cmds[i][4] == '0') {
                            std::cout << "\nSystem is switched off";

                            std::string reply = "Recieved S 0 0 Command";
                            serv.send_string(reply);
                        }

                        else {
                            std::string reply = "Got some other message";
                            serv.send_string(reply);
                        }
                    }

                    //Sort Commands
                    else if (cmds[i][2] == '1') {
                        if (cmds[i][4] == '1') {
                            std::cout << "\nSent Material to BIN2";

                            std::string reply = "Recieved S 1 1 Command";
                            serv.send_string(reply);
                        }

                        else if (cmds[i][4] == '0') {
                            std::cout << "\nSent material to BIN1";

                            std::string reply = "Recieved S 1 0 Command";
                            serv.send_string(reply);
                        }

                        else {
                            std::string reply = "Got some other message";
                            serv.send_string(reply);
                        }
                    }

                    else {
                        std::string reply = "Got some other message";
                        serv.send_string(reply);
                    }
                }

                //Get commands
                else if (cmds[i][0] == 'G') {

                    //System status command
                    if (cmds[i][2] == '0') {

                        std::cout << "\nSYSTEM STATUS IS PLACEHOLDER";

                        std::string reply = "SYSTEM IS PLACEHOLDER";
                        serv.send_string(reply);

                    }

                    //BIN Count Commands
                    else if (cmds[i][2] == '1') {

                        //BIN1
                        if (cmds[i][4] == '0') {
                            std::cout << "\nBIN1 count is PLACEHOLDER";

                            std::string reply = "PLACEHOLDER";
                            serv.send_string(reply);
                        }

                        //BIN2
                        else if (cmds[i][4] == '1') {
                            std::cout << "\nBIN2 count is PLACEHOLDER";

                            std::string reply = "PLACEHOLDR";
                            serv.send_string(reply);
                        }

                        else {
                            std::string reply = "Got some other message";
                            serv.send_string(reply);
                        }
                    }

                    ////Get real time image
                    //else if (cmds[i][2] == '2') {
                    //    video_live = true;
                    //    std::string reply = "WebCam Operational";
                    //    serv.send_string(reply);
                    //}

                    else {
                        std::string reply = "Got some other message";
                        serv.send_string(reply);
                    }
                }

                else {
                    std::string reply = "Got some other message";
                    serv.send_string(reply);
                }
            }
        }

        /*if (video_live) {

            if (vid.isOpened() == true)
            {
                cv::Mat frame;
                vid >> frame;
                if (frame.empty() == false)
                {
                    imshow("Server Image", frame);
                    process_msg();
                    serv.set_txim(frame);
                }
            }
        }*/
    } while (serverthreadexit == false);

    serv.stop();

    Sleep(100);
}


int main(int argc, char* argv[])
{
	int cmd = -1;
	do
	{
		print_menu();
		std::cin >> cmd;
		switch (cmd)
		{
        case 1: test_joystick(); break;
        case 2: toggle_led(); break;
        case 3: get_button(); break;
        case 4: rotate_servo(); break;
        case 5: etch_sketch(); break;
        case 6: pong(); break;
        case 7: spaceInvaders(); break;
        case 8: test_sprite(); break;
        case 9: do_video(); break;
        case 10: server(); break;
		}
	} while (cmd != 0);
}
