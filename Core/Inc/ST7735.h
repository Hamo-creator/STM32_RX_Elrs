#ifndef __ST7735_H
#define __ST7735_H

#include "stm32f4xx_hal.h"
#include "fonts.h"

// Define your SPI handler here
extern SPI_HandleTypeDef hspi2;
#define ST7735_SPI_PORT hspi2

// Define your CS, DC, and RST pins here
#define CS_PORT GPIOA
#define CS_PIN  GPIO_PIN_8
#define AD_PORT GPIOB
#define AD_PIN  GPIO_PIN_12
#define RST_PORT GPIOB
#define RST_PIN  GPIO_PIN_14

// TFT Defines
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128

// Colors
#define BLACK         0x0000
#define NAVY          0x000F
#define DARKGREEN     0x03E0
#define DARKCYAN      0x03EF
#define MAROON        0x7800
#define PURPLE        0x780F
#define OLIVE         0x7BE0
#define LIGHTGREY     0xC618
#define DARKGREY      0x7BEF
#define BLUE          0x001F
#define GREEN         0x07E0
#define CYAN          0x07FF
#define RED           0xF800
#define MAGENTA       0xF81F
#define YELLOW        0xFFE0
#define WHITE         0xFFFF
#define ORANGE        0xFD20
#define GREENYELLOW   0xAFE5
#define PINK          0xF81F

// Font structures (placeholders, actual font data would be in font files)
/*typedef struct {
    const uint8_t *data;
    uint8_t width;
    uint8_t height;
    uint8_t first_char;
    uint8_t last_char;
} FontDef;

extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;*/

// Function prototypes (based on typical ST7735 libraries)
void ST7735_Init(uint8_t orientation);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bg_color);
void ST7735_SetRotation(uint8_t rotation);
void ST7735_FillScreen(uint16_t color);

// DMA specific function (will be implemented in ST7735.c)
void ST7735_SPI_Transmit_DMA(uint8_t *data, uint32_t size);

#endif // __ST7735_H


