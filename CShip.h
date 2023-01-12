#pragma once
#include "stdafx.h"
#include "CGameObject.h"

/**
*
* @brief Class for player ship object
*
* Child class of CGameObject and is used to create a player ship for
* Space Invaders
*
* @author Tristan Huen
*
*/
class CShip : public CGameObject {
private:
	int _width; ///< Width of the ship
	int _height; ///< Height of the ship
public:

	/** @brief Default constructor for player ship. Only used for creation before assignment.
	*
	* @return nothing to return
	*/
	CShip();

	/** @brief Initializes the player's ship
	*
	* @param ship_size The dimension of the ship
	* @param lives Amount of player lives to start with
	* @param velocity The velocity of ship (only horizontal movement)
	*
	* @return nothing to return
	*/
	CShip(cv::Size2f ship_size, int lives, cv::Point2f velocity);

	/** @brief Gets width of ship
	*
	* @return Ship width
	*/
	int get_width() { return _width; }

	/** @brief Gets height of ship
	*
	* @return Ship height
	*/
	int get_height() { return _height; }

	/** @brief Gets velocity of ship
	*
	* @return Ship velocity
	*/
	cv::Point2f get_velocity() { return _velocity; }

	/** @brief Sets velocity of ship
	*
	* @param velocity The desired velocity to be set to
	* @return nothing to return
	*/
	void set_velocity(cv::Point2f velocity) { _velocity = velocity; }

	/** @brief Custom draw function for ship 
	*
	* @return nothing to return
	*/
	void draw(cv::Mat& im);

};
