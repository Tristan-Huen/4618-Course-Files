#include "stdafx.h"
#include "CSpaceInvaderGame.h"
#include <cstdint>
#include <random>

#define JOY_X_CHAN 11

#define HIGH 1
#define LOW 0

#define DIGITAL 0

#define SW1 33
#define SW2 32

#define LIVES 3
#define POINTS 10

#define INVADERS_COL_NUM 5
#define INVADERS_ROW_NUM 11

#define MAX_SCORE (INVADERS_COL_NUM * INVADERS_ROW_NUM * POINTS)

#define MISSILE_WIDTH 3
#define MISSILE_HEIGHT 25
#define MISSILE_VELOCITY 700

#define MAX_MISSILES 3

#define SHIP_WIDTH 65
#define SHIP_HEIGHT 25
#define SHIP_VELOCITY 250

#define INVADER_WIDTH 40
#define INVADER_HEIGHT 20

#define INVADER_HORZ_VELOCITY 600
#define INVADER_VERT_VELOCITY 700

#define INVADER_HORZ_SPACING 60
#define INVADER_VERT_SPACING 40

#define DEBOUNCE_TIME 50

//Tank in actual game is 13 pixels wide and 5 pixels tall (except cannon portion)
//Main Invader is 11 pixels wide and 8 pixels tall
//Big Squid Invader is 12 pixels wide and 8 pixels tall
//Tiny Squid Invader is 8 pixels wide and 8 pixels tall
//The grid of invaders is 11 wide and 5 tall
//Special red invader is 16 wide and 7 tall


/* TO DO
1) Make last alien go light speed
2) Red alien ship
3) Ship sprite
4) Explosion effects
5) Accurate text
6) Different missile sprites
7) Ship destruction sprite
8) Barriers ? (Actual pain)
*/

CSpaceInvaderGame::CSpaceInvaderGame(int comport, cv::Size canvas_size) {
	port.init_com(comport);
	_canvas = cv::Mat::zeros(canvas_size, CV_8UC3);
    init();
}

void CSpaceInvaderGame::init() {
    set_reset_state(false);

    set_score(0);

    _canvas = cv::Mat::zeros(_canvas.size(), CV_8UC3);

    _animation = ANIMATION_ONE;
    _time_to_draw = 700;
    
    _invaders.clear();
    _missiles.clear();
    _enemy_missiles.clear();

    _ship = CShip(cv::Size2f(SHIP_WIDTH, SHIP_HEIGHT), LIVES, cv::Point2f(0, 0));

    //Create invaders.
    for (int column_count = 0; column_count < INVADERS_COL_NUM; column_count++) {
        for (int invader_index = 0; invader_index < INVADERS_ROW_NUM; invader_index++) {
            if (column_count == 0) {
                CInvader invader = CInvader(cv::Point2f(180 + invader_index * INVADER_HORZ_SPACING, 700 - 610 + column_count * INVADER_VERT_SPACING), INV_TINY);
                invader.set_velocity(cv::Point2f(INVADER_HORZ_VELOCITY, 0));
                _invaders.push_back(invader);
            }

            else if (column_count >= 3) {
                CInvader invader = CInvader(cv::Point2f(180 + invader_index * INVADER_HORZ_SPACING, 700 - 610 + column_count * INVADER_VERT_SPACING), INV_BIG);
                invader.set_velocity(cv::Point2f(INVADER_HORZ_VELOCITY, 0));
                _invaders.push_back(invader);
            }

            else {
                CInvader invader = CInvader(cv::Point2f(180 + invader_index * INVADER_HORZ_SPACING, 700 - 610 + column_count * INVADER_VERT_SPACING), INV_MAIN);
                invader.set_velocity(cv::Point2f(INVADER_HORZ_VELOCITY, 0));
                _invaders.push_back(invader);
            }
            
        }
    }

    _start = std::chrono::system_clock::now();
}

void CSpaceInvaderGame::update() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> rand_missile(-1, 1);

	float joy_x_val;
    float direction;

    int accel = 1;

    if (_invaders.size() == 26) {
        accel = 1.5;
        _time_to_draw = 350;
    }

    else if (_invaders.size() == 13) {
        accel = 1.5;
        _time_to_draw = 175;
    }

    else if (_invaders.size() == 6) {
        accel = 1.5;
        _time_to_draw = 100;
    }

    //Still need to construct probability thing and then set lives to one 
    if (_special_invader.get_lives() > 0) {
        _special_invader = CSpecialInvader(cv::Point2f(0 - _special_invader.get_width(), _special_invader.get_height() + 10));
        _special_invader.set_velocity(cv::Point(INVADER_HORZ_VELOCITY * accel, 0));
    }

    port.get_analog(JOY_X_CHAN, joy_x_val, port);

    if (joy_x_val > 0.55 || joy_x_val < 0.50) {
        direction = (joy_x_val > 0.53) ? 1 : -1;
    }

    else {
        direction = 0;
    }
   
    _ship.set_velocity(cv::Point2f(SHIP_VELOCITY, 0.0) * direction);
	_ship.move();

    //If the ship collides with the walls bound it
    if (_ship.collide_wall(_canvas.size())) {
        if (_ship.get_pos().x <= 0) {
            _ship.set_pos(cv::Point2f(0, _ship.get_pos().y));
        }

        if (_ship.get_pos().x + _ship.get_width() >= _canvas.size().width) {
            _ship.set_pos(cv::Point2f(_canvas.size().width- _ship.get_width(), _ship.get_pos().y));
        }
    }

    //Move the missiles
    for (int missiles_index = 0; missiles_index < _missiles.size(); missiles_index++) {
        _missiles[missiles_index].move();
    }

    //Move the enemy missiles
    for (int enemy_missiles_index = 0; enemy_missiles_index < _enemy_missiles.size(); enemy_missiles_index++) {
        _enemy_missiles[enemy_missiles_index].move();
    }

    //Check for missile collisions with wall and invaders.
    for (int missiles_index = 0; missiles_index < _missiles.size(); missiles_index++) {
        
        if (_missiles[missiles_index].collide_wall(_canvas.size())) {
            _missiles.erase(_missiles.begin() + missiles_index);
        }

        else if (_missiles[missiles_index].collide(_special_invader)) {
            _special_invader.set_lives(0);
            set_score(get_score() + POINTS * 5);
        }

        else if (_missiles.size() > 0) {
            for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                if (_missiles.size() > 0 && _missiles[missiles_index].collide(_invaders[invader_index])) {
                    _missiles.erase(_missiles.begin() + missiles_index);
                    _invaders[invader_index].hit();
                    set_score(get_score() + POINTS);
                    break; //If this isn't here then vector subscript errors can happen.
                }
            }
        }
    }

    //Check for enemy missile collisions with wall and player
    for (int enemy_missiles_index = 0; enemy_missiles_index < _enemy_missiles.size(); enemy_missiles_index++) {

        if (_enemy_missiles[enemy_missiles_index].collide_wall(_canvas.size())) {
            _enemy_missiles.erase(_enemy_missiles.begin() + enemy_missiles_index);
            break;
        }
        
        if (_enemy_missiles[enemy_missiles_index].collide(_ship)) {
            _ship.set_lives(_ship.get_lives() - 1);
            _enemy_missiles.erase(_enemy_missiles.begin() + enemy_missiles_index);
            break;
        }

    }

    //Check which invaders are dead and erase them
    for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
        if (_invaders[invader_index].get_lives() == 0) {
            _invaders.erase(_invaders.begin() + invader_index);
        }
    }

    /*
    Create a rect starting at obj and ending vertically downwards at some point past the last row of invaders
    (which moves based on invader). If this rect intersects any of the objs then it is blocked.
    */

    for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
        cv::Rect2f cast_ray = cv::Rect2f(_invaders[invader_index].get_pos(), 
            cv::Size2f(_invaders[invader_index].get_width(), INVADER_VERT_SPACING * 5));
        int block_count = 0;
        for (int invader_index_two = 0; invader_index_two < _invaders.size(); invader_index_two++) {

            //Check if invader is being blocked
            if ((_invaders[invader_index_two].get_pos().y > cast_ray.y &&
                _invaders[invader_index_two].get_pos().y < cast_ray.y + cast_ray.height &&
                _invaders[invader_index_two].get_pos().x >= cast_ray.x - INVADER_VERT_SPACING * 0.5 &&
                _invaders[invader_index_two].get_pos().x <= cast_ray.x + cast_ray.width + INVADER_VERT_SPACING * 0.5)) {
                block_count++;
            }
        }

        //If invader is not blocked then fire a missile
        if (block_count == 0 && rand_missile(rng) > 0) { 
            if (_enemy_missiles.size() < 1) {
                CMissile missile = CMissile(cv::Size2f(MISSILE_WIDTH, MISSILE_HEIGHT),
                    _invaders[invader_index].get_pos() + cv::Point2f(_invaders[invader_index].get_width() * 0.5, 30), cv::Point2f(0, MISSILE_VELOCITY));
                _enemy_missiles.push_back(missile);
            }
        }
    }

    ////Move invaders across screen
   
    auto end = std::chrono::system_clock::now();

    //if enough frame time has elapsed then move the invaders
    if (std::chrono::duration_cast<std::chrono::milliseconds>(end - _start).count() >= _time_to_draw) {
        _start = std::chrono::system_clock::now();
        _animation *= -1;


        for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {

            //If invaders hit right wall then go down and go left
            if (_invaders[invader_index].collide_wall(_canvas.size()) &&
                _invaders[invader_index].get_pos().x + INVADER_WIDTH >= _canvas.size().width) {
                for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                    _invaders[invader_index].set_velocity(cv::Point2f(0, INVADER_VERT_VELOCITY *accel));
                    _invaders[invader_index].move();
                    _invaders[invader_index].set_velocity(cv::Point2f(-INVADER_HORZ_VELOCITY * accel, 0));
                }
                break;
            }

            //If invaders hit left wall then go down and go right
            else if (_invaders[invader_index].collide_wall(_canvas.size()) &&
                _invaders[invader_index].get_pos().x <= _canvas.size().width) {
                for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
                    _invaders[invader_index].set_velocity(cv::Point2f(0, INVADER_VERT_VELOCITY * accel));
                    _invaders[invader_index].move();
                    _invaders[invader_index].set_velocity(cv::Point2f(INVADER_HORZ_VELOCITY * accel, 0));
                }
                break;
            }
        }

        for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
            _invaders[invader_index].move();
        }

    }

    //If invaders get to close to bottom of screen then player loses
    for (int invader_index = 0; invader_index < _invaders.size(); invader_index++) {
        if (_invaders[invader_index].get_pos().y + _invaders[invader_index].get_height() >= 700-50) {
            _ship.set_lives(0);
            break;
        }
    }

    //As long as special invader exists then move it across the screen
    if (_special_invader.get_lives() > 0) {
        if (_special_invader.get_pos().x > _canvas.size().width) {
            _special_invader.set_lives(0);
        }
        else {
            _special_invader.move();
        }
    }

    button_fire(SW1);
    button_reset(SW2);

    if (_score >= MAX_SCORE || _ship.get_lives() <= 0) {
        set_reset_state(true);
    }
}

void CSpaceInvaderGame::draw() {

    if (get_reset_state()) {
        
        if (_ship.get_lives() <= 0) {
            _canvas = cv::Mat::zeros(cv::Size(_canvas.size().width, _canvas.size().height), CV_8UC3);
            cv::putText(_canvas, "You Lost", cv::Point(_canvas.size().width * 0.5 - 60, _canvas.size().height * 0.5+20),
                cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);

        }

        else if (_invaders.empty()) {
            _canvas = cv::Mat::zeros(cv::Size(_canvas.size().width, _canvas.size().height), CV_8UC3);
            cv::putText(_canvas, "You Won!", cv::Point(_canvas.size().width * 0.5 - 60, _canvas.size().height * 0.5 + 20),
                cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
        }

        button_reset(SW2);
        cv::imshow("Space Invaders", _canvas);
    }

    else {
        _canvas = cv::Mat::zeros(cv::Size(_canvas.size().width, _canvas.size().height), CV_8UC3);
        _ship.draw(_canvas);

        //Draw all missiles
        for (auto missiles : _missiles) {
            missiles.draw(_canvas);
        }

        //Draw all enemy missiles
        for (auto enemy_missiles : _enemy_missiles) {
            enemy_missiles.draw(_canvas);
        }

        //Draw all invaders
        for (auto invaders: _invaders) {
            invaders.draw(_animation, _canvas);
        }

        //Draw special invader if its alive
        if (_special_invader.get_lives() > 0) {
            _special_invader.draw(_canvas);
        }

        cv::putText(_canvas, "Score: " + std::to_string(_score),
            cv::Point(_canvas.size().width / 4 - 60, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

        cv::putText(_canvas, "Missiles: " + std::to_string(MAX_MISSILES -_missiles.size()),
            cv::Point(_canvas.size().width * 0.5 - 60, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

        cv::putText(_canvas, "Lives: " + std::to_string(_ship.get_lives()),
            cv::Point(_canvas.size().width / 2 + _canvas.size().width / 4 - 60, 30), cv::FONT_HERSHEY_COMPLEX, 1.0,
            cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

        cv::imshow("Space Invaders", _canvas);
    }
	
}

void CSpaceInvaderGame::button_fire(int channel) {
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
                if (_missiles.size() < MAX_MISSILES) {
                    CMissile missile = CMissile(cv::Size2f(MISSILE_WIDTH, MISSILE_HEIGHT),
                        _ship.get_pos() + cv::Point2f(_ship.get_width() * 0.5, -30), cv::Point2f(0, -MISSILE_VELOCITY));
                    _missiles.push_back(missile);
                }
           
            }
        }
    }
}

void CSpaceInvaderGame::button_reset(int channel) {
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
                set_reset_state(false);
                init();
            }
        }
    }
}