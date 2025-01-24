#include "i2c-lcd.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_COMMAND 0x00
#define LCD_DATA 0x01

static void lcd_send_command(I2C_HandleTypeDef *hi2c, uint8_t cmd);
static void lcd_send_data(I2C_HandleTypeDef *hi2c, uint8_t data);
static void lcd_send(I2C_HandleTypeDef *hi2c, uint8_t data, uint8_t flags);

void lcd_init(I2C_HandleTypeDef *hi2c)
{

    HAL_Delay(50);
    lcd_send_command(hi2c, 0x30); // Wake up
    HAL_Delay(5);
    lcd_send_command(hi2c, 0x30);
    HAL_Delay(1);
    lcd_send_command(hi2c, 0x30);
    HAL_Delay(10);

    lcd_send_command(hi2c, 0x20); // Set 4-bit mode
    HAL_Delay(1);

    // Display settings
    lcd_send_command(hi2c, 0x28); // 4-bit mode, 2-line, 5x8 font
    lcd_send_command(hi2c, 0x08); // Display off
    lcd_send_command(hi2c, 0x01); // Clear display
    HAL_Delay(2);
    lcd_send_command(hi2c, 0x06); // Entry mode set, increment automatically
    lcd_send_command(hi2c, 0x0C); // Display on, cursor off
}

void lcd_send_string(I2C_HandleTypeDef *hi2c, char *str)
{
    for (int i = 0; i < 16 && *str != '\0'; i++)
    {
        lcd_send_data(hi2c, (uint8_t)(*str++));
    }

    if (*str != '\0')
    {
        lcd_send_command(hi2c, 0xC0);

        for (int i = 0; i < 16 && *str != '\0'; i++)
        {
            lcd_send_data(hi2c, (uint8_t)(*str++));
        }
    }
}

void lcd_clear(I2C_HandleTypeDef *hi2c)
{
    lcd_send_command(hi2c, 0x01); // Clear display
    HAL_Delay(2);
}

void lcd_display_message_with_float(I2C_HandleTypeDef *hi2c, const char *message, float value)
{
    char buffer[32]; // Buffer to hold the formatted string

    // Format the string with the float value
    snprintf(buffer, sizeof(buffer), message, value);

    lcd_clear(hi2c);        // Clear the LCD
    lcd_send_string(hi2c, buffer); // Display the formatted string
}

static void lcd_send_command(I2C_HandleTypeDef *hi2c, uint8_t cmd)
{
    lcd_send(hi2c, cmd, LCD_COMMAND);
}

static void lcd_send_data(I2C_HandleTypeDef *hi2c, uint8_t data)
{
    lcd_send(hi2c, data, LCD_DATA);
}

static void lcd_send(I2C_HandleTypeDef *hi2c, uint8_t data, uint8_t flags)
{
    uint8_t upper_nibble = (data & 0xF0) | LCD_BACKLIGHT | flags;
    uint8_t lower_nibble = ((data << 4) & 0xF0) | LCD_BACKLIGHT | flags;

    uint8_t data_arr[4];
    data_arr[0] = upper_nibble | LCD_ENABLE;
    data_arr[1] = upper_nibble & ~LCD_ENABLE;
    data_arr[2] = lower_nibble | LCD_ENABLE;
    data_arr[3] = lower_nibble & ~LCD_ENABLE;

    HAL_I2C_Master_Transmit(hi2c, LCD_ADDR, data_arr, 4, 100);
}

void lcd_display_two_floats(I2C_HandleTypeDef *hi2c, char *line1, char *line2, float value1, float value2)
{
    char buffer[16]; // Buffer for formatted strings, max 16 characters per line

    // Clear the LCD before displaying new content
    lcd_clear(hi2c);

    // Display the first line
    if (line1 != NULL)
    {
        snprintf(buffer, sizeof(buffer), line1, value1);
        lcd_send_command(hi2c, 0x80); // Set cursor to the beginning of the first line
        lcd_send_string(hi2c, buffer);
    }

    // Display the second line
    if (line2 != NULL)
    {
        snprintf(buffer, sizeof(buffer), line2, value2);
        lcd_send_command(hi2c, 0xC0); // Set cursor to the beginning of the second line
        lcd_send_string(hi2c, buffer);
    }
}

// Example usage in the main function or elsewhere:
// float humidity = 45.67;
// lcd_display_message_with_float(&hi2c1, "Humidity: %.2f%%", humidity);
