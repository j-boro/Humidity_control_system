/*
 * serial_connection.h
 *
 *  Created on: Jan 25, 2025
 *      Author: mccp3
 */

#ifndef INC_SERIAL_CONNECTION_H_
#define INC_SERIAL_CONNECTION_H_

#include "stm32f7xx_hal.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
float receiveData();


#endif /* INC_SERIAL_CONNECTION_H_ */
