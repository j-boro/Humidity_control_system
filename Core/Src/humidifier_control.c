/*
 * humidifier_control.c
 *
 *  Created on: Jan 23, 2025
 *      Author: Bobo
 */
#include "humidifier_control.h"
#include "gpio.h"
#include "stm32f7xx_hal.h"

extern int state;

void humidifier_on(){
	if(state == 0){
		  HAL_GPIO_WritePin(TOUCH_GPIO_Port, TOUCH_Pin, 1);
		  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(TOUCH_GPIO_Port, TOUCH_Pin, 0);
		  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 0);
		  state++;
	}
}

void humidifier_off(){
	  if(state == 1 || state == 2){
		  HAL_GPIO_WritePin(TOUCH_GPIO_Port, TOUCH_Pin, 1);
		  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(TOUCH_GPIO_Port, TOUCH_Pin, 0);
		  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 0);
		  state++;
		  if(state == 3)
			  state = 0;
	  }
}


