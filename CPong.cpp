#include "stdafx.h"
#include "CPong.h"
#include <string>
#include <thread>
#include <chrono>
#include <random>

#define JOY_Y_CHAN 4

#define PADDLE_MIDDLE 700;

#define PADDLE_HEIGHT 175
#define HALF_PADDLE_HEIGHT (PADDLE_HEIGHT / 2)
#define PADDLE_LEFT_EDGE 960
#define PADDLE_RIGHT_EDGE 980
#define PADDLE_WIDTH (PADDLE_RIGHT_EDGE - PADDLE_LEFT_EDGE)

#define MAX_BALL_VELOCITY 450.0
#define MIN_BALL_VELOCITY 400.0

#define FPS 60.00

#define HIGH 1
#define LOW 0

#define DIGITAL 0

#define SW1 33

#define DEBOUNCE_TIME 50

bool CPong::_reset = false;

std::mutex frame_mutex;

void signofNum(const int &num, int &sign) {
    if (num < 0) {
        sign = -1;
    }

    else {
        sign = 1;
    }
}

void setMinVelocity(cv::Point2f& velocity, const float &min_speed) {
    int velocity_sign;

    if (abs(velocity.x) < min_speed || velocity.x == 0) {
        signofNum(velocity.x, velocity_sign);
        velocity.x = min_speed;
        velocity.x *= velocity_sign;
    }

    if (abs(velocity.y) < min_speed || velocity.y == 0) {
        signofNum(velocity.y, velocity_sign);
        velocity.y = min_speed;
        velocity.y *= velocity_sign;
    }
}

CPong::CPong(int comport, cv::Size canvas_size) {
    port.init_com(comport);
    _canvas = cv::Mat::zeros(canvas_size, CV_8UC3);
}

void CPong::update() {

    frame_mutex.lock();
    const cv::Point2f screen_middle = cv::Point2f(_canvas.size().width / 2.0, _canvas.size().height / 2.0);

    frame_mutex.unlock();
    
    //Code snippet from: https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c][]
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> rand_velocity(-MAX_BALL_VELOCITY, MAX_BALL_VELOCITY);

    cv::Point2f random_velocity = cv::Point2f(rand_velocity(rng), rand_velocity(rng));

    static cv::Point2f old_position = _position;

    _end_draw_time = std::chrono::system_clock::now();

    frame_mutex.lock();
    float delta_t = (std::chrono::duration_cast<std::chrono::milliseconds>(_start_draw_time - _end_draw_time).count()) / 1000.0;
    frame_mutex.unlock();

    _start_draw_time = std::chrono::system_clock::now();

    get_button(SW1, true, &set_reset_state);

    if (get_reset_state()) {
        set_reset_state(false);

        frame_mutex.lock();
        _position = screen_middle;
        _velocity = random_velocity;

        _player_one_score = 0;
        _player_two_score = 0;

        frame_mutex.unlock();
    }

    if (_player_one_score >= 5 || _player_two_score >= 5) {
        frame_mutex.lock();
        _position = screen_middle;
        _velocity = random_velocity;

        frame_mutex.unlock();
        set_reset_state(true);

        do {
            get_button(SW1, false, &set_reset_state);
        } while (get_reset_state());

        frame_mutex.lock();
        _player_one_score = 0;
        _player_two_score = 0;
        frame_mutex.unlock();
    }

    float joy_y_val;

    frame_mutex.lock();
    float enemy_pos = _position.y;
    frame_mutex.unlock();

    port.get_analog(JOY_Y_CHAN, joy_y_val, port);

    unsigned int paddle_pos = (1 - joy_y_val) * PADDLE_MIDDLE;

    //Use joystick to track middle of paddle
    frame_mutex.lock();
    _joystick = cv::Point((PADDLE_LEFT_EDGE + PADDLE_RIGHT_EDGE) / 2, paddle_pos);
    frame_mutex.unlock();

    //If paddle goes above screen bound it
    if ((_joystick.y - HALF_PADDLE_HEIGHT) < 0) {
        _joystick.y = HALF_PADDLE_HEIGHT;
    }
    //If paddle goes below screen bound it
    else if ((_joystick.y + HALF_PADDLE_HEIGHT) > _canvas.size().height) {
        _joystick.y = _canvas.size().height - HALF_PADDLE_HEIGHT;
    }

    if ((enemy_pos - HALF_PADDLE_HEIGHT) < 0) {
        enemy_pos = HALF_PADDLE_HEIGHT;
    }

    else if ((enemy_pos + HALF_PADDLE_HEIGHT) > _canvas.size().height) {
        enemy_pos = _canvas.size().height - HALF_PADDLE_HEIGHT;
    }

    //Top left vertex and bottom right vertex of enemy paddle
    frame_mutex.lock();
    _enemy_paddle_one = cv::Point(_canvas.size().width - PADDLE_RIGHT_EDGE, enemy_pos - HALF_PADDLE_HEIGHT);
    _enemy_paddle_two = cv::Point(_canvas.size().width - PADDLE_LEFT_EDGE, enemy_pos + HALF_PADDLE_HEIGHT);
    frame_mutex.unlock();

	//Top left vertex and bottom right vertex of paddle
    frame_mutex.lock();
	_rect_vert_one = cv::Point(PADDLE_LEFT_EDGE, _joystick.y - HALF_PADDLE_HEIGHT);
	_rect_vert_two = cv::Point(PADDLE_RIGHT_EDGE, _joystick.y + HALF_PADDLE_HEIGHT);
    frame_mutex.unlock();

    //If ball is outside y bounds of canvas
    frame_mutex.lock();
    if (_position.y >= _canvas.size().height - BALL_RADIUS || _position.y <= BALL_RADIUS) {

        //If ball is outside max height then lock it back to max height
        if (_position.y >= _canvas.size().height - BALL_RADIUS) {
            _position.y = _canvas.size().height - BALL_RADIUS;
            _velocity.y *= -1;
        }

        //If ball is outside min height then lock it back to min height
        else if (_position.y <= BALL_RADIUS) {
            _position.y = BALL_RADIUS;
            _velocity.y *= -1;
        }
        
    }

    //If ball goes past player 1 or 2 boundary
    if (_position.x >= _canvas.size().width + BALL_RADIUS || _position.x < -BALL_RADIUS) {

        if (_position.x >= _canvas.size().width + BALL_RADIUS) {
            _player_one_score++;
            _position = screen_middle;
            _velocity = random_velocity;
        }

        else if (_position.x < -BALL_RADIUS) {
            _player_two_score++;
            _position = screen_middle;
            _velocity = random_velocity;
        }

    }

    //If ball hits left or right paddle of player 2
    if (old_position.y >= _rect_vert_one.y && old_position.y <= _rect_vert_two.y &&
        (_position + _velocity * delta_t).x >= (_rect_vert_one.x - BALL_RADIUS) &&
        (_position + _velocity * delta_t).x <= (_rect_vert_two.x + BALL_RADIUS)) {

        if (_position.x - old_position.x > 0) {
            _position.x = _rect_vert_one.x - BALL_RADIUS;

        }

        if (_position.x - old_position.x < 0) {
            _position.x = _rect_vert_two.x + BALL_RADIUS;
        }

        _velocity.x *= -1;
    }

    //If ball hits top or bottom paddle of player 2
    if (old_position.x >= _rect_vert_one.x && old_position.x <= _rect_vert_two.x &&
        (_position + _velocity * delta_t).y >= (_rect_vert_one.y- BALL_RADIUS) &&
        (_position + _velocity * delta_t).y <= (_rect_vert_two.y+ BALL_RADIUS)) {

        if (_position.y - old_position.y > 0) {
            _position.y = _rect_vert_one.y - BALL_RADIUS;
        }

        if (_position.y - old_position.y < 0) {
            _position.y = _rect_vert_two.y + BALL_RADIUS;
        }

        _velocity.y *= -1;
    }

    //If ball hits left or right paddle of enemy player
    if (old_position.y >= _enemy_paddle_one.y && old_position.y <= _enemy_paddle_two.y &&
        (_position + _velocity * delta_t).x >= (_enemy_paddle_one.x- BALL_RADIUS) &&
        (_position + _velocity * delta_t).x <= (_enemy_paddle_two.x+ BALL_RADIUS)) {

        if (_position.x - old_position.x > 0) {
            _position.x = _enemy_paddle_one.x - BALL_RADIUS;
        }

        if (_position.x - old_position.x < 0) {
            _position.x = _enemy_paddle_two.x + BALL_RADIUS;
        }

        _velocity.x *= -1;
    }

    //If ball hits top or bottom paddle of enemy player
    if (old_position.x >= _enemy_paddle_one.x && old_position.x <= _enemy_paddle_two.x &&
        (_position + _velocity * delta_t).y >= (_enemy_paddle_one.y- BALL_RADIUS) &&
        (_position + _velocity * delta_t).y <= (_enemy_paddle_two.y+ BALL_RADIUS)) {

        if (_position.y - old_position.y > 0) {
            _position.y = _enemy_paddle_one.y - BALL_RADIUS;
        }

        if (_position.y - old_position.y < 0) {
            _position.y = _enemy_paddle_two.y + BALL_RADIUS;
        }

        _velocity.y *= -1;
    }
    
    frame_mutex.unlock();

    if (abs(_velocity.x) < MIN_BALL_VELOCITY || abs(_velocity.y) < MIN_BALL_VELOCITY ||
        _velocity.y == 0 || _velocity.x == 0 ) {
        setMinVelocity(_velocity, MIN_BALL_VELOCITY);
    }

    frame_mutex.lock();
    _position += _velocity * delta_t;
    frame_mutex.unlock();

    old_position = _position;

}

void CPong::draw() {

    frame_mutex.lock();
    _canvas = cv::Mat::zeros(cv::Size(_canvas.size().width, _canvas.size().height), CV_8UC3);
    frame_mutex.unlock();

    frame_mutex.lock();
	cv::putText(_canvas, "Player 1:" + std::to_string(_player_one_score),
        cv::Point(_canvas.size().width / 4 - 60, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
		cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    frame_mutex.unlock();

    frame_mutex.lock();
	cv::putText(_canvas, "Player 2:" + std::to_string(_player_two_score),
        cv::Point(_canvas.size().width / 2 + _canvas.size().width / 4 - 60, 30),  cv::FONT_HERSHEY_COMPLEX, 1.0,
		cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    frame_mutex.unlock();

    cv::putText(_canvas, "FPS: " + std::to_string(_fps),
        cv::Point(_canvas.size().width / 2 + _canvas.size().width / 4 - 60, 700-30), cv::FONT_HERSHEY_COMPLEX, 1.0,
        cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

    frame_mutex.lock();
	cv::rectangle(_canvas, _rect_vert_one, _rect_vert_two, cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
    frame_mutex.unlock();

    frame_mutex.lock();
    cv::rectangle(_canvas, _enemy_paddle_one, _enemy_paddle_two, cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
    frame_mutex.unlock();

    frame_mutex.lock();
    cv::circle(_canvas, _position, BALL_RADIUS, cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
    frame_mutex.unlock();

	cv::line(_canvas, cv::Point(_canvas.size().width/2, 0), cv::Point(_canvas.size().width/2,  _canvas.size().height),
        cv::Scalar(128, 128, 128), 1, cv::LINE_AA);

	cv::imshow("Pong", _canvas);

}

void CPong::run() {
   
    std::thread t1(&CPong::update_thread, this);
    std::thread t2(&CPong::draw_thread, this);
    t1.join();
    t2.join();
   
}

void CPong::update_thread(CPong* ptr) {
    while (cv::waitKey(10) != 'q') {
        ptr->update();

        //Code snippet from: https://stackoverflow.com/questions/38730273/how-to-limit-fps-in-a-loop-with-c
        ptr->_end_frame = std::chrono::system_clock::now();

        std::chrono::duration<float, std::milli> work_time = ptr->_end_frame - ptr->_start_frame;

        if (work_time.count() < 25.0) {
            std::chrono::duration<float, std::milli> delta_ms(25.0 - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(delta_ms_duration);
        }

        ptr->_start_frame = std::chrono::system_clock::now();
        std::chrono::duration<float, std::milli> sleep_time = ptr->_start_frame - ptr->_end_frame;

        ptr->_fps = 1 / ((work_time + sleep_time) / 1000.0).count();
    }
}

void CPong::draw_thread(CPong* ptr) {
    while (cv::waitKey(10) != 'q') {
        ptr->draw();
    }

}


bool CPong::get_reset_state() {
	return _reset;
}

void CPong::set_reset_state(bool flag_state) {
	_reset = flag_state;
}

void CPong::get_button(int channel, bool flag_state, static void (*function)(bool)) {
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