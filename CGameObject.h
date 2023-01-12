#pragma once
#include "stdafx.h"

/**
*
* @brief Base class for all game objects (missiles, invaders, ship)
*
* The base class for the Space Invaders game which contains a general
* template for all the objects instantiated in the game.
*
* @author Tristan Huen
*
*/
class CGameObject {
protected:
	cv::Point2f _position; ///< Position of the object on the screen
	cv::Point2f _velocity; ///< Velocity of the object
	cv::Rect2f _shape; ///< The dimensions of the objects rectangle
	int _lives; ///< Number of lives

public:

	/** @brief Moves the game object based on velocity and time since move() was last called.
	*
	* @return nothing to return
	*/
	void move();

	/** @brief Detects if the object collides with another object.
	*
	* @param obj The other object to check if they collide.
	* @return true if collision detected, false otherwise.
	*/
	bool collide(CGameObject& obj);

	/** @brief Detects if an object collides with the walls of container.
	*
	* @param board The canvas size
	* @return true if collision is detected, false otherwise. 
	*/
	bool collide_wall(cv::Size board);

	/** @brief Removes a life from the object.
	*
	* @param comport The serial com port
	* @param canvas_size Dimensions of the drawing screen
	* @return nothing to return
	*/
	void hit();

	/** @brief Get the current number of lives of the object.
	*
	* @return Current number of lives of the object
	*/
	int get_lives() { return _lives; }

	/** @brief Set the current life of the object.
	*
	* @return nothing to return
	*/
	void set_lives(int lives) { _lives = lives; }

	/** @brief Set the current position of the object.
	*
	* @return nothing to return
	*/
	void set_pos(cv::Point2f pos) { _position = pos; }

	/** @brief Get the current position of the object.
	*
	* @return Current position of object
	*/
	cv::Point2f get_pos() { return _position; }

	/** @brief Draws the object to the screen
	*
	* @param im Canvas being drawn on
	* @return nothing to return
	*/
	void draw(cv::Mat& im);
};