#include "stdafx.h"
#include "CInvader.h"

#define SCALE_FACTOR 3
#define PIX_HEIGHT 8
#define MAIN_PIX_WIDTH 11
#define BIG_PIX_WIDTH 12
#define TINY_PIX_WIDTH 8

int CInvader::sprite_main[] = { 
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
    0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
    0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
    0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
};

int CInvader::sprite_main_two[] = { 
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
    1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
    1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
    1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
    0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
    0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
    0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
};

int CInvader::sprite_big[] = {
        0,0,0,0,1,1,1,1,0,0,0,0,  // ....@@@@....
        0,1,1,1,1,1,1,1,1,1,1,0,  // .@@@@@@@@@@.
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        1,1,1,0,0,1,1,0,0,1,1,1,  // @@@..@@..@@@
        1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
        0,0,0,1,1,0,0,1,1,0,0,0,  // ...@@..@@...
        0,0,1,0,0,1,1,0,0,1,0,0,  // ..@..@@..@..
        0,0,0,1,0,0,0,0,1,0,0,0   // ...@....@...
};

int CInvader::sprite_big_two[] = { 
    0,0,0,0,1,1,1,1,0,0,0,0,  // ....@@@@.... 
    0,1,1,1,1,1,1,1,1,1,1,0,  // .@@@@@@@@@@.
    1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
    1,1,1,0,0,1,1,0,0,1,1,1,  // @@@..@@..@@@
    1,1,1,1,1,1,1,1,1,1,1,1,  // @@@@@@@@@@@@
    0,0,0,1,1,0,0,1,1,0,0,0,  // ...@@..@@...
    0,0,1,1,0,1,1,0,1,1,0,0,  // ..@@.@@.@@..
    1,1,0,0,0,0,0,0,0,0,1,1   // @@........@@
};

int CInvader::sprite_tiny[] = {
        0,0,0,1,1,0,0,0,          // ...@@...
        0,0,1,1,1,1,0,0,          // ..@@@@..
        0,1,1,1,1,1,1,0,          // .@@@@@@.
        1,1,0,1,1,0,1,1,          // @@.@@.@@
        1,1,1,1,1,1,1,1,          // @@@@@@@@
        0,0,1,0,0,1,0,0,          // ..@..@.. 
        0,1,0,0,0,0,1,0,          // .@....@.
        0,0,1,0,0,1,0,0           // ..@..@..
};

int CInvader::sprite_tiny_two[] = {
    0,0,0,1,1,0,0,0,          // ...@@...
    0,0,1,1,1,1,0,0,          // ..@@@@..
    0,1,1,1,1,1,1,0,          // .@@@@@@.
    1,1,0,1,1,0,1,1,          // @@.@@.@@
    1,1,1,1,1,1,1,1,          // @@@@@@@@
    0,0,1,0,0,1,0,0,          // ..@..@.. 
    0,1,0,1,1,0,1,0,          // .@.@@.@.
    1,0,1,0,0,1,0,1           // @.@..@.@
};

CInvader::CInvader(cv::Point2f initial_pos, int sprite_type) {
    _position = initial_pos;
    set_lives(1);

    switch (sprite_type) {
    case INV_MAIN:
        _sprite_type = INV_MAIN;
        _shape = cv::Rect2f(_position, cv::Size2f(MAIN_PIX_WIDTH * SCALE_FACTOR, PIX_HEIGHT * SCALE_FACTOR));
        break;
    case INV_BIG:
        _sprite_type = INV_BIG;
        _shape = cv::Rect2f(_position, cv::Size2f(BIG_PIX_WIDTH * SCALE_FACTOR, PIX_HEIGHT * SCALE_FACTOR));
        break;
    case INV_TINY:
        _sprite_type = INV_TINY;
        _shape = cv::Rect2f(_position, cv::Size2f(TINY_PIX_WIDTH * SCALE_FACTOR, PIX_HEIGHT * SCALE_FACTOR));
        break;
    }

}

void CInvader::draw(int animation, cv::Mat& im) {

    if (_sprite_type == INV_MAIN) {
        if (animation == ANIMATION_ONE) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < MAIN_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_main[(rows + MAIN_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == ANIMATION_TWO) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < MAIN_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_main_two[(rows + MAIN_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }
        }
    }

    else if (_sprite_type == INV_BIG) {
        if (animation == ANIMATION_ONE) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < BIG_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_big[(rows + BIG_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == ANIMATION_TWO) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < BIG_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_big_two[(rows + BIG_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }
    }

    else if (_sprite_type == INV_TINY) {
        if (animation == ANIMATION_ONE) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < TINY_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_tiny[(rows + TINY_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }

        else if (animation == ANIMATION_TWO) {
            for (int col = 0; col < PIX_HEIGHT; col++) {
                for (int rows = 0; rows < TINY_PIX_WIDTH; rows++) {
                    cv::rectangle(im, _position + cv::Point2f(rows * SCALE_FACTOR, col * SCALE_FACTOR),
                        _position + cv::Point2f((rows + 1) * SCALE_FACTOR, (col + 1) * SCALE_FACTOR),
                        sprite_tiny_two[(rows + TINY_PIX_WIDTH * col)] * cv::Scalar(255, 255, 255), cv::FILLED, cv::LINE_AA);
                }
            }

        }
    }

    
}