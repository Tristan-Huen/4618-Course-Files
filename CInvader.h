#pragma once
#include "stdafx.h"
#include "CGameObject.h"

enum {INV_MAIN = 1, INV_BIG, INV_TINY};

#define ANIMATION_ONE 1
#define ANIMATION_TWO -1

/**
*
* @brief Class for invader object
*
* Child class of CGameObject and is used to create an invader for
* Space Invaders
*
* @author Tristan Huen
*
*/
class CInvader : public CGameObject {
private:

    int _sprite_type; ///< Type of sprite being drawn.

    static int sprite_main[88]; ///< Array containing first animation frame of the main sprite.
    static int sprite_main_two[88];  ///< Array containing second animation frame of the main sprite.
    static int sprite_big[96]; ///< Array containing first animation frame of the big squid sprite.
    static int sprite_big_two[96]; ///< Array containing second animation frame of the big squid sprite.
    static int sprite_tiny[64]; ///< Array containing first animation frame of the tiny squid sprite.
    static int sprite_tiny_two[64]; ///< Array containing second animation frame of the tiny squid sprite.
public:

    /** @brief Initializes an invader
	*
	* @param initial_pos Initial position of invader
	*
	* @return nothing to return
	*/
    CInvader(cv::Point2f initial_pos, int sprite_type);

    /** @brief Draws the invader to the screen
    *
    * @param animation The current frame animation
    * @param im Canvas being drawn on
    * @return nothing to return
    */
    void draw(int animation, cv::Mat& im);

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
