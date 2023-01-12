#pragma once

#include "stdafx.h"
#include "CControl.h"

/**
*
* @brief Base class for all labs in 4618
*
* Contains pure virtual methods for use in child classes for different
* labs
*
* @author Tristan Huen
*
*/
class CBase4618 {
protected:
	CControl port; ///< CControl port object
	cv::Mat _canvas; ///< OpenCV matrix object

public:
	/** @brief Pure virtual method to update any internal variables
	*
	* @return nothing to return
	*/
	virtual void update() = 0;

	/** @brief Pure virtual method to draw to screen
	*
	* @return nothing to return
	*/
	virtual void draw() = 0;

	/** @brief Runs update and draw in loop until 'q' is hit
	*
	* @return nothing to return
	*/
	virtual void run();
};