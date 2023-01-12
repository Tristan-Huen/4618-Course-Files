#pragma once
#include "stdafx.h"
#include "CGameObject.h"

/**
*
* @brief Class for missile object
*
* Child class of CGameObject and is used to create a missile for
* Space Invaders
*
* @author Tristan Huen
*
*/
class CMissile : public CGameObject {
public:

	/** @brief Instantiates a missile object
	*
	* @param missile_size Dimensions of the missile
	* @param current_pos The current position of the missile
	* @param velocity Velocity of missile
	* @return nothing to return
	*/
	CMissile(cv::Size2f missile_size, cv::Point2f current_pos, cv::Point2f velocity);
};