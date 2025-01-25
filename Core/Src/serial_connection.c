/*
 * serial_connection.c
 *
 *  Created on: Jan 25, 2025
 *      Author: mccp3
 */

#include "serial_connection.h"
#include "stm32f7xx_hal.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define RX_BUFFER_SIZE 64

extern char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t data_received = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        static uint16_t rx_index = 0;

        if (rx_buffer[rx_index] == '\n')
        {
            rx_buffer[rx_index] = '\0';
            data_received = 1;
            rx_index = 0;
        }
        else
        {
            rx_index = (rx_index + 1) % RX_BUFFER_SIZE;
        }

        HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_buffer[rx_index], 1);
    }
}

float receiveData()
{
    if (data_received)
    {
        data_received = 0;

        rx_buffer[strcspn(rx_buffer, "\r\n")] = '\0';


        char *end_ptr;
        float value = strtof(rx_buffer, &end_ptr);

        if (end_ptr != rx_buffer && *end_ptr == '\0' && value >= 0.0f && value <= 100.0f)
        {
            return value;
        }
    }

    return -1.0f;
}
