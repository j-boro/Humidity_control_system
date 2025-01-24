#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "stm32f7xx_hal.h"

#define LCD_ADDR (0x27 << 1) // Change this if your LCD uses a different address
#define LCD_ROWS 2
#define LCD_COLS 16

void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_send_string(I2C_HandleTypeDef *hi2c, char *str);
void lcd_clear(I2C_HandleTypeDef *hi2c);
void lcd_display_message_with_float(I2C_HandleTypeDef *hi2c, const char *message, float value);
void lcd_display_two_floats(I2C_HandleTypeDef *hi2c, char *line1, char *line2, float value1, float value2);


#endif
