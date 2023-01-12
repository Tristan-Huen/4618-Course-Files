#pragma once
#include "stdafx.h"
#include "CBase4618.h"
#include "CShip.h"
#include "CInvader.h"
#include "CMissile.h"
#include "CSpecialInvader.h"
#include <vector>

/**
*
* @brief Class for Space Invaders Game
*
* Child class of CBase4618 and is used to play Space Invaders on screen using
* the Boosterpack joystick and buttons
*
* @author Tristan Huen
*
*/
class CSpaceInvaderGame : public CBase4618 {
private:
	int _score; ///< Score of player
	int _animation; ///< Animation of the sprite to play
	int _time_to_draw; ///< Time till invaders will be next drawn
	bool _reset_flag; ///< Flag to reset game (true for reset)
	CShip _ship; ///< Player ship 
	CSpecialInvader _special_invader; ///< Special invader
	std::vector<CInvader> _invaders; ///< Vector of invaders
	std::vector<CMissile> _missiles; ///< Vector of missiles
	std::vector<CMissile> _enemy_missiles; ///< Vector of enemy missiles
	std::chrono::system_clock::time_point _start; ///< Time point for animating sprite
public:

	/** @brief Initializes serial com port and screen size
	*
	* @param comport The serial com port
	* @param canvas_size Dimensions of the play screen
	* @return nothing to return
	*/
	CSpaceInvaderGame(int comport = 3, cv::Size canvas_size = cv::Size(1000, 700));
	
	/** @brief Initializes starting state with invaders and score
	*
	* @return nothing to return
	*/
	void init();

	/** @brief Updates internal variables such as positions and lives of player, invaders and missiles.
	*
	* @return nothing to return
	*/
	void update();

	/** @brief Draws missiles, ship, invaders, etc.
	*
	* @return nothing to return
	*/
	void draw();

	/** @brief Fires a missile if a button has been pressed.
	*
	* @param channel Pin of the button pressed
	* @return nothing to return
	*/
	void button_fire(int channel);

	/** @brief Sets reset flag if button is pressed
	*
	* @param channel Pin of the button pressed
	* @return nothing to return
	*/
	void button_reset(int channel);

	/** @brief Sets the score to a given value.
	* 
	* @param score The value to set the current score to
	*
	* @return nothing to return
	*/
	void set_score(int score) { _score = score; }

	/** @brief Gets the current score
	*
	* @return Returns current score
	*/
	int get_score() { return _score; }

	/** @brief Gets the current state of reset flag
	*
	* @return Returns the reset flag state
	*/
	bool get_reset_state() { return _reset_flag; }

	/** @brief Sets the reset flag to desired state
	*
	* @param flag_state State of the reset flag
	* @return nothing to return
	*/
	void set_reset_state(bool flag_state) { _reset_flag = flag_state; }
};
