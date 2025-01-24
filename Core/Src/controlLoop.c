/*
 * controlLoop.c
 *
 *  Created on: Jan 24, 2025
 *      Author: mccp3
 */

extern _Bool tryingFor, fanState;
_Bool uncert = 0;

void uncertain(float humidity, float lowerThresh, float upperThresh){
	if(humidity < lowerThresh || humidity > upperThresh){
		uncert = 0;
	}
}

void main_control_loop(float humidity, float lowerThresh, float upperThresh){

	if(uncert == 0){
		if(humidity < lowerThresh){
			tryingFor = 1;
		} else if(humidity > upperThresh){
			tryingFor = 0;
		} else if(humidity >= lowerThresh && humidity <= upperThresh && tryingFor == 1){
			uncert = 1;
			tryingFor = 0;
		} else if(humidity >= lowerThresh && humidity <= upperThresh && tryingFor == 0){
			uncert = 1;
			tryingFor = 1;
		}
	} else if(uncert == 1)
		uncertain(humidity, lowerThresh, upperThresh);

}

void fan_control(float humidity, float lowerThresh, float upperThresh){
	if(humidity > upperThresh){
		fanState = 1;
	} else if(humidity < lowerThresh){
		fanState = 0;
	}
}
