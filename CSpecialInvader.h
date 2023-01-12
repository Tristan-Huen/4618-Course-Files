#pragma once
#include "CGameObject.h"

/**
*
* @brief Class for special invader object
*
* Child class of CGameObject and is used to create an special invader for
* Space Invaders
*
* @author Tristan Huen
*
*/
class CSpecialInvader : public CGameObject {
private:
	static int sprite[112]; ///< Sprite for special invader
public: 

	/** @brief Default constructor for special invader. Only used for creation before assignment.
	*
	* @return nothing to return
	*/
	CSpecialInvader::CSpecialInvader();

	/** @brief Initializes a special invader
	*
	* @param initial_pos Initial position of special invader
	*
	* @return nothing to return
	*/
	CSpecialInvader(cv::Point2f initial_pos);

	/** @brief Draws the invader to the screen
	*
	* @param animation The current frame animation
	* @param im Canvas being drawn on
	* @return nothing to return
	*/
	void draw(cv::Mat& im);

	/** @brief Gets velocity of invader
	*
	* @return Invader velocity
	*/
	cv::Point get_velocity() { return _velocity; }

	/** @brief Sets velocity of invader
	*
	* @param velocity The desired velocity to be set to
	* @return nothing to return
	*/
	void set_velocity(cv::Point velocity) { _velocity = velocity; }

	/** @brief Gets width of ship
	*
	* @return Ship width
	*/
	int get_width() { return _shape.width; }

	/** @brief Gets height of ship
	*
	* @return Ship height
	*/
	int get_height() { return _shape.height; }
};

