/*
 * humidifier_config.c
 *
 *  Created on: Jan 31, 2025
 *      Author: mccp3
 */

#include "humidifier_config.h"
#include "stm32f7xx_hal.h"
#include "i2c.h"

float readHumidity(void) {
  uint8_t command[2] = {0xFD};
  uint8_t data[6];
  HAL_StatusTypeDef ret;

  ret = HAL_I2C_Master_Transmit(&hi2c1, 0x44 << 1, command, 1, 1000);
  if (ret != HAL_OK) {
    return -1;
  }

  HAL_Delay(10);

  ret = HAL_I2C_Master_Receive(&hi2c1, 0x44 << 1, data, 6, 1000);
  if (ret != HAL_OK) {
    return -1;
  }

  uint16_t rawHumidity = (data[3] << 8) | data[4];
  float humidity = -6.0 + 125.0 * (float)rawHumidity / 65535.0;

  return humidity;
}
