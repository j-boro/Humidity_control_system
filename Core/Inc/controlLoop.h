/*
 * controlLoop.h
 *
 *  Created on: Jan 24, 2025
 *      Author: mccp3
 */

#ifndef INC_CONTROLLOOP_H_
#define INC_CONTROLLOOP_H_

void uncertain(float humidity, float lowerThresh, float upperThresh);
void main_control_loop(float humidity, float lowerThresh, float upperThresh);
void fan_control(float humidity, float lowerThresh, float upperThresh);


#endif /* INC_CONTROLLOOP_H_ */
