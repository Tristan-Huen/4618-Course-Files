#pragma once
#include "stdafx.h"
#include "Serial.h"

/**
* 
* @brief Provides digital, analog and servo read/write
* 
* Provides a serial connection to the com port and allows reading/writing
* to the microcontroller.
* 
* @author Tristan Huen
* 
*/
class CControl {
private:
	Serial _com; ///< Com port object 
public:
	/** @brief Initializes com port serial communication
	*
	* @param comport The com port being used by the microcontroller
	* @return nothing to return
	*/
	void init_com(int comport);

	/** @brief Gets data from a channel/pin on the com port
	*
	* @param type The type of the data (DIGITAL, ANALOG or SERVO)
	* @param channel The pin/channel being used
	* @param result The data received from the port
	* @return true if command succeeded and false if command timed out
	*/
	bool get_data(int type, int channel, int& result);

	/** @brief Sets a channel/pin on the com port
	*
	* @param type The type of the data (DIGITAL or SERVO)
	* @param channel The pin/channel being used
	* @param value The value the channel should be set to
	* @return true if command succeeded and false if command timed out
	*/
	bool set_data(int type, int channel, int value);

	/** @brief Gets an analog value as a percent of the max value on ADC
	*
	* @param channel The pin/channel being read
	* @param result_percent Percent of the max ADC value on the channel
	* @param Port object to get analog data
	* @return true if command succeeded and false if command timed out
	*/
	void get_analog(int channel, float &result_percent, CControl &port);
};
