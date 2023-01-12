#pragma once
#include "stdafx.h"
#include "CBase4618.h"

/**
*
* @brief Class for Etch-A-Sketch game
*
* Child class of CBase4618 and is used to play Etch-A-Sketch on screen using
* the Boosterpack joystick and buttons
*
* @author Tristan Huen
*
*/
class CSketch : public CBase4618 {
private:
	cv::Point _point; ///< Point object to track current joystick position
	static int _color; ///< Index to cycle through colors
	static bool _reset; ///< Flag for reset (set when true)
public: 

	/** @brief Initializes serial com port and drawing screen size
	*
	* @param comport The serial com port
	* @param canvas_size Dimensions of the drawing screen
	* @return nothing to return
	*/
	CSketch(int comport=3, cv::Size canvas_size = cv::Size(1000,600));

	/** @brief Gets analog joystick values in terms of screen position
	*
	* @return nothing to return
	*/
	void get_analog();

	/** @brief Updates internal variables of class
	*
	* @return nothing to return
	*/
	void update();

	/** @brief Draws user controlled line on screen
	*
	* @return nothing to return
	*/
	void draw();

	/** @brief Executes a function using a debounced pushbutton
	*
	* @param channel Pin of switch/pushbutton
	* @param (*function)() Function to perform action if button pressed
	* @return nothing to return
	*/
	void get_button(int channel, void (*function)());

	/** @brief Executes a flag function using a debounced pushbutton
	*
	* @param channel Pin of switch/pushbutton
	* @param flag state Flag to be set once button is pressed
	* @param (*function)(bool) Function to set a flag if button pressed
	* @return nothing to return
	*/
	void get_button(int channel, bool flag_state, static void (*function)(bool));

	/** @brief Gets current index of color cycle
	*
	* @return Index of color cycle
	*/
	static int get_color();

	/** @brief Increments current index of color cycle
	*
	* @return nothing to return
	*/
	static void increment_color();

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
};