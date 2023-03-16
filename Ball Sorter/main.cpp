////////////////////////////////////////////////////////////////
// ELEX 4618 Template project for BCIT
// Created Oct 5, 2016 by Craig Hennessey
// Last updated Dec 6, 2021
////////////////////////////////////////////////////////////////

// Add simple GUI elements
#define CVUI_DISABLE_COMPILATION_NOTICES
#define CVUI_IMPLEMENTATION

#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>
#include "pigpio.h"


#include "CControlPi.h"
#include "CPong.h"
#include "CRecyclingSort.h"
#include "server.h"

#define CANVAS_NAME "Display Image"

#define HIGH 1
#define LOW 0

#define DEBOUNCE_TIME 10

#define COM3 3

#define MAX_SERVO_ANGLE 2200
#define MIN_SERVO_ANGLE 600

#define SW1 33

#define PIN2 2
#define PIN3 3
#define PIN6 17

#define JOY_Y_CHAN 16
#define JOY_X_CHAN 0

#define MAX_ADC_VAL 1023

enum { RGBLED_BLU_PIN = 37, RGBLED_GRN_PIN, RGBLED_RED_PIN };

void test_joystick() {
    cv::Mat canvas = cv::Mat::zeros(cv::Size(100,100),CV_8UC3);

    CControlPi port;

    port.init_com(3);

    float joy_x_percent = 0;
    float joy_y_percent = 0;

    std::cout.precision(3);

    do {

        //Possibly use this keyword?
        port.get_analog(JOY_X_CHAN, joy_x_percent, port);
        port.get_analog(JOY_Y_CHAN, joy_y_percent, port);

        std::cout << "ANALOG TEST: CH11 X = " << (int)(joy_x_percent * MAX_ADC_VAL) << "(" << joy_x_percent * 100.0
            << "%) CH4 Y = " << (int)(joy_y_percent * MAX_ADC_VAL)  << "(" << joy_y_percent * 100.0 << "%)\n";

        cv::imshow("C",canvas);

    } while (cv::waitKey(10) != 'q');

}

void toggle_led() {
    cv::Mat canvas = cv::Mat::zeros(cv::Size(100,100),CV_8UC3);

    CControlPi port;

    port.init_com(COM3);

    int button_state = LOW;

    do {

        port.get_data(DIGITAL, PIN2, button_state);

        if (button_state == LOW) {
            port.set_data(DIGITAL, PIN3, HIGH);
            std::cout << "DIGITAL TEST: CH38 = " << HIGH << "\n";
        }

        else {
            port.set_data(DIGITAL, PIN3, LOW);
            std::cout << "DIGITAL TEST: CH38 = " << LOW << "\n";
        }

        cv::imshow("C",canvas);

    } while (cv::waitKey(10) != 'q');

}


void get_button() {

    cv::Mat canvas = cv::Mat::zeros(cv::Size(100,100),CV_8UC3);

    CControlPi port;

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

        port.get_data(DIGITAL, PIN2, reading);

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

        cv::imshow("C",canvas);

    } while (cv::waitKey(10) != 'q');

}


void rotate_servo() {

    cv::Mat canvas = cv::Mat::zeros(cv::Size(100,100),CV_8UC3);

    CControlPi port;

    port.init_com(COM3);

    do {


        //Rotate from min angle to max angle
        for (int angle = MIN_SERVO_ANGLE; angle <= MAX_SERVO_ANGLE; angle += 10) {
            std::cout << "SERVO TEST CH: " << angle << "\n";
            port.set_data(SERVO, PIN6, angle);
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Slow down servo rotate speed
        }

        //Rotate from max angle to min angle
        for (int angle = MAX_SERVO_ANGLE; angle >= MIN_SERVO_ANGLE; angle -= 10) {
            std::cout << "SERVO TEST CH: " << angle << "\n";
            port.set_data(SERVO, PIN6, angle);
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); //Slow down servo rotate speed
        }

        cv::imshow("C",canvas);

    } while (cv::waitKey(10) != 'q');

}

void pong() {
   CPong game = CPong();
   game.run();
}

void recycling() {
    CRecyclingSort recycle_sort = CRecyclingSort();
    recycle_sort.run();
}

void get_button(int channel, bool &is_pressed, CControlPi &port) {
    static int button_state = 1;
    static int last_button_state = LOW;

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

            if(button_state == LOW) {
                is_pressed = true;
            }

            else {
                is_pressed = false;
            }
        }
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

    vid.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    vid.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    if (vid.isOpened() == true)
    {

        do
        {
            cv::Mat frame;
            vid >> frame;
            if (frame.empty() == false)
            {
                imshow("Server Image", frame);
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
    CControlPi port;
    port.init_com(3);

    int bin_one_count = 0;
    int bin_two_count = 0;

    bool system_on = false;

    port.set_data(DIGITAL, 2, 0);
    port.set_data(SERVO, 3, 1500);

    char inputchar;
    bool video_live = false;
    std::vector<std::string> cmds;

    cv::VideoCapture vid;

    vid.open(0);

    vid.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    vid.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    //Unknown freezing and preventing q to quit
    //std::thread t1(&serverimagethread);
    //t1.detach();


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
                            port.set_data(DIGITAL, 2, 1);
                            system_on = true;

                            std::string reply = "Recieved: System is switched on";
                            serv.send_string(reply);

                        }

                        else if (cmds[i][4] == '0') {
                            std::cout << "\nSystem is switched off";
                            port.set_data(DIGITAL, 2, 0);
                            system_on = false;

                            std::string reply = "Recieved: System is switched off";
                            serv.send_string(reply);
                        }

                        else {
                            std::string reply = "Got some other message";
                            serv.send_string(reply);
                        }
                    }

                    //Sort Commands
                    else if (cmds[i][2] == '1') {
                        if(system_on) {
                             std::string reply = "ERROR: System is still on can't run manual ";
                             serv.send_string(reply);
                        }

                        else {
                            if (cmds[i][4] == '1') {
                            std::cout << "\nSent Material to BIN2";
                            port.set_data(SERVO, 3, 2200);
                             std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            port.set_data(SERVO, 3, 1500);
                            bin_two_count++;

                            std::string reply = "Recieved: Sent Material to BIN2";
                            serv.send_string(reply);
                            }

                            else if (cmds[i][4] == '0') {
                            std::cout << "\nSent material to BIN1";
                            port.set_data(SERVO, 3, 800);
                             std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            port.set_data(SERVO, 3, 1500);
                            bin_one_count++;

                            std::string reply = "Recieved: Sent material to BIN1";
                            serv.send_string(reply);
                            }

                            else {
                            std::string reply = "Got some other message";
                            serv.send_string(reply);
                            }

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

                        std::string system_status;

                        if(system_on) {
                            system_status = "ON";
                        }

                        else {
                            system_status = "OFF";
                        }

                        std::cout << "\nSYSTEM IS " + system_status;

                        std::string reply = "Recieved: SYSTEM IS " + system_status;
                        serv.send_string(reply);

                    }

                    //BIN Count Commands
                    else if (cmds[i][2] == '1') {

                        std::string bin_one = std::to_string(bin_one_count);
                        std::string bin_two = std::to_string(bin_two_count);

                        //BIN1
                        if (cmds[i][4] == '0') {
                             std::cout << "\nBIN1 count is " + bin_one;

                             std::string reply = "BIN1 count is " + bin_one;
                             serv.send_string(reply);
                        }

                        //BIN2
                        else if (cmds[i][4] == '1') {
                             std::cout << "\nBIN2 count is " + bin_two;

                             std::string reply = "BIN2 count is " + bin_two;
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

                else {
                    std::string reply = "Got some other message";
                    serv.send_string(reply);
                }
            }
        }


            if (vid.isOpened() == true)
            {

                cv::Mat frame;
                vid >> frame;


                if (frame.empty() == false)
                {
                    imshow("Server Image", frame);
                    //process_msg();
                    serv.set_txim(frame);
                }
            }

    } while (serverthreadexit == false);

    serv.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void print_menu()
{
	std::cout << "\n***********************************";
	std::cout << "\n* ELEX4618 Template Project";
	std::cout << "\n***********************************";
    std::cout << "\n(1) Test Joystick";
    std::cout << "\n(2) Test Digital I/O";
    std::cout << "\n(3) Test Debounced Button";
    std::cout << "\n(4) Test Servo Rotation";
    std::cout << "\n(5) Pong";
    std::cout << "\n(6) Test Recycling";
    std::cout << "\n(7) Test Networking";
	std::cout << "\n(0) Exit";
	std::cout << "\nCMD> ";
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
        case 5: pong(); break;
        case 6: recycling(); break;
        case 7: server(); break;
		}
	} while (cmd != 0);
}
