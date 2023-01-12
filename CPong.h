#pragma once
#include "stdafx.h"
#include "CBase4618.h"

#include <thread>

#define BALL_RADIUS 10.0

/**
*
* @brief Class for Pong game
*
* Child class of CBase4618 and is used to play Pong on screen using
* the Boosterpack joystick and buttons
*
* @author Tristan Huen
*
*/
class CPong : public CBase4618 {
private:
	int _player_one_score = 0; ///< Score of player one
	int _player_two_score = 0; ///< Score of player two
	float _fps; ///< Current FPS 
	bool _thread_exit = true;
	cv::Point _joystick; ///< Point object to track current joystick position
	cv::Point2f _velocity = cv::Point2f(400.0, 450.0); ///< Velocity of ball
	cv::Point2f _position = cv::Point2f(500.0, 350.0); ///< Position of ball
	cv::Point _rect_vert_one; ///< Top left corner of paddle
	cv::Point _rect_vert_two; ///< Bottom right corner of paddle
	cv::Point _enemy_paddle_one; ///< Top left corner of enemy paddle
	cv::Point _enemy_paddle_two; ///< Bottom right corner of paddle
	std::chrono::system_clock::time_point _start_frame = std::chrono::system_clock::now(); ///< Start of a frame
	std::chrono::system_clock::time_point _end_frame = std::chrono::system_clock::now(); ///< End of a frame
	std::chrono::system_clock::time_point _start_draw_time = std::chrono::system_clock::now(); ///< Start of draw time
	std::chrono::system_clock::time_point _end_draw_time = std::chrono::system_clock::now();  ///< End of draw time
	static bool _reset; ///< Flag for reset (set when true)
public:

	/** @brief Initializes serial com port and drawing screen size
	*
	* @param comport The serial com port
	* @param canvas_size Dimensions of the drawing screen
	* @return nothing to return
	*/
	CPong(int comport = 3, cv::Size canvas_size = cv::Size(1000, 700));


	/** @brief Updates internal variables such as paddle position, ball position and collisions.
	*
	* @return nothing to return
	*/
	void update();

	/** @brief Draws paddles, ball, score and FPS on screen
	*
	* @return nothing to return
	*/
	void draw();
	
	/** @brief Multithreaded version of run with update and draw as seperate threads running until 'q' is hit
	* @return nothing to return
	*/
	void run();
	
	/** @brief Updates the update thread and locks the framerate to 30 FPS
	* 
	* @param ptr Pointer to current CPong object
	* @return nothing to return
	*/
	static void update_thread(CPong* ptr);

	/** @brief Updates the draw thread
	* 
	* @param ptr Pointer to current CPong object
	* @return nothing to return
	*/
	static void draw_thread(CPong* ptr);

	/** @brief Gets current state of reset flag
	*
	* @return true if flag is set, false otherwise
	*/
	static bool get_reset_state();

	/** @brief Sets the reset flag to desired state
	*
	* @param flag_state State of the reset flag
	* @return nothing to return
	*/
	static void set_reset_state(bool flag_state);

	/** @brief Executes a flag function using a debounced pushbutton
	*
	* @param channel Pin of switch/pushbutton
	* @param flag state Flag to be set once button is pressed
	* @param (*function)(bool) Function to set a flag if button pressed
	* @return nothing to return
	*/
	void get_button(int channel, bool flag_state, static void (*function)(bool));

};
