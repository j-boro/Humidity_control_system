/*
 * serial_connection.c
 *
 *  Created on: Jan 25, 2025
 *      Author: mccp3
 */

#include "serial_connection.h"
#include "stm32f7xx_hal.h"
#include "usart.h"
#include "crc.h"
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

        //data parsing
        char *delimiter = strrchr(rx_buffer, ',');
        if (!delimiter)
            return -1.0f;

        //data splitting
        *delimiter = '\0';
        uint16_t received_crc = (uint16_t)atoi(delimiter + 1);

        uint16_t computed_crc = compute_crc16((uint8_t *)rx_buffer, strlen(rx_buffer));
        if (received_crc != computed_crc)
            return -1.0f; // CRC mismatch

        char *end_ptr;
        float value = strtof(rx_buffer, &end_ptr);

        if (end_ptr != rx_buffer && *end_ptr == '\0' && value >= 0.0f && value <= 100.0f)
        {
            return value;
        }
    }

    return -1.0f;
}
