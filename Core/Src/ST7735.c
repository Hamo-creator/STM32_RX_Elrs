#include "ST7735.h"
#include <string.h>

// Static variables for DMA transfer
static volatile uint8_t dma_transfer_complete = 1;

// DMA Transfer Complete Callback
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == ST7735_SPI_PORT.Instance)
    {
        dma_transfer_complete = 1;
    }
}

// Wait for DMA transfer to complete
static void ST7735_WaitForDMA(void)
{
    while (!dma_transfer_complete)
    {
        // Wait for DMA transfer to complete
    }
}

// DMA-based SPI transmit function
void ST7735_SPI_Transmit_DMA(uint8_t *data, uint32_t size)
{
    dma_transfer_complete = 0;
    HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT, data, size);
    ST7735_WaitForDMA();
}

// Basic SPI transmit function (blocking)
static void ST7735_SPI_Transmit(uint8_t *data, uint32_t size)
{
    HAL_SPI_Transmit(&ST7735_SPI_PORT, data, size, HAL_MAX_DELAY);
}

// Write command to ST7735
static void ST7735_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(AD_PORT, AD_PIN, GPIO_PIN_RESET); // Command mode
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET); // Select device
    ST7735_SPI_Transmit(&cmd, 1);
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);   // Deselect device
}

// Write data to ST7735
static void ST7735_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(AD_PORT, AD_PIN, GPIO_PIN_SET);   // Data mode
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET); // Select device
    ST7735_SPI_Transmit(&data, 1);
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);   // Deselect device
}

// Write data buffer to ST7735 using DMA
static void ST7735_WriteDataDMA(uint8_t *data, uint32_t size)
{
    HAL_GPIO_WritePin(AD_PORT, AD_PIN, GPIO_PIN_SET);   // Data mode
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET); // Select device
    ST7735_SPI_Transmit_DMA(data, size);
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);   // Deselect device
}

// Set address window
static void ST7735_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column address set
    ST7735_WriteCommand(0x2A);
    ST7735_WriteData(0x00);
    ST7735_WriteData(x0);
    ST7735_WriteData(0x00);
    ST7735_WriteData(x1);

    // Row address set
    ST7735_WriteCommand(0x2B);
    ST7735_WriteData(0x00);
    ST7735_WriteData(y0);
    ST7735_WriteData(0x00);
    ST7735_WriteData(y1);

    // Write to RAM
    ST7735_WriteCommand(0x2C);
}

// Initialize ST7735
void ST7735_Init(uint8_t orientation)
{
    // Reset the display
    HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(RST_PORT, RST_PIN, GPIO_PIN_SET);
    HAL_Delay(5);

    // Software reset
    ST7735_WriteCommand(0x01);
    HAL_Delay(150);

    // Out of sleep mode
    ST7735_WriteCommand(0x11);
    HAL_Delay(500);

    // Frame rate control - normal mode
    ST7735_WriteCommand(0xB1);
    ST7735_WriteData(0x01);
    ST7735_WriteData(0x2C);
    ST7735_WriteData(0x2D);

    // Frame rate control - idle mode
    ST7735_WriteCommand(0xB2);
    ST7735_WriteData(0x01);
    ST7735_WriteData(0x2C);
    ST7735_WriteData(0x2D);

    // Frame rate control - partial mode
    ST7735_WriteCommand(0xB3);
    ST7735_WriteData(0x01);
    ST7735_WriteData(0x2C);
    ST7735_WriteData(0x2D);
    ST7735_WriteData(0x01);
    ST7735_WriteData(0x2C);
    ST7735_WriteData(0x2D);

    // Display inversion control
    ST7735_WriteCommand(0xB4);
    ST7735_WriteData(0x07);

    // Power control
    ST7735_WriteCommand(0xC0);
    ST7735_WriteData(0xA2);
    ST7735_WriteData(0x02);
    ST7735_WriteData(0x84);
    ST7735_WriteCommand(0xC1);
    ST7735_WriteData(0xC5);

    ST7735_WriteCommand(0xC2);
    ST7735_WriteData(0x0A);
    ST7735_WriteData(0x00);

    ST7735_WriteCommand(0xC3);
    ST7735_WriteData(0x8A);
    ST7735_WriteData(0x2A);
    ST7735_WriteCommand(0xC4);
    ST7735_WriteData(0x8A);
    ST7735_WriteData(0xEE);

    // VCOM
    ST7735_WriteCommand(0xC5);
    ST7735_WriteData(0x0E);

    // Display inversion off
    ST7735_WriteCommand(0x20);

    // Memory access control
    ST7735_WriteCommand(0x36);
    ST7735_WriteData(0xC8);

    // Pixel format
    ST7735_WriteCommand(0x3A);
    ST7735_WriteData(0x05);

    // Gamma correction
    ST7735_WriteCommand(0xE0);
    ST7735_WriteData(0x02);
    ST7735_WriteData(0x1C);
    ST7735_WriteData(0x07);
    ST7735_WriteData(0x12);
    ST7735_WriteData(0x37);
    ST7735_WriteData(0x32);
    ST7735_WriteData(0x29);
    ST7735_WriteData(0x2D);
    ST7735_WriteData(0x29);
    ST7735_WriteData(0x25);
    ST7735_WriteData(0x2B);
    ST7735_WriteData(0x39);
    ST7735_WriteData(0x00);
    ST7735_WriteData(0x01);
    ST7735_WriteData(0x03);
    ST7735_WriteData(0x10);

    ST7735_WriteCommand(0xE1);
    ST7735_WriteData(0x03);
    ST7735_WriteData(0x1D);
    ST7735_WriteData(0x07);
    ST7735_WriteData(0x06);
    ST7735_WriteData(0x2E);
    ST7735_WriteData(0x2C);
    ST7735_WriteData(0x29);
    ST7735_WriteData(0x2D);
    ST7735_WriteData(0x2E);
    ST7735_WriteData(0x2E);
    ST7735_WriteData(0x37);
    ST7735_WriteData(0x3F);
    ST7735_WriteData(0x00);
    ST7735_WriteData(0x00);
    ST7735_WriteData(0x02);
    ST7735_WriteData(0x10);

    // Normal display on
    ST7735_WriteCommand(0x13);
    HAL_Delay(10);

    // Display on
    ST7735_WriteCommand(0x29);
    HAL_Delay(100);

    ST7735_SetRotation(orientation);
}

// Draw a single pixel
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;

    ST7735_SetAddressWindow(x, y, x, y);

    uint8_t data[] = {color >> 8, color & 0xFF};
    ST7735_WriteDataDMA(data, 2);
}

// Fill rectangle with color using DMA
void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if ((x >= ST7735_WIDTH) || (y >= ST7735_HEIGHT)) return;
    if ((x + w - 1) >= ST7735_WIDTH) w = ST7735_WIDTH - x;
    if ((y + h - 1) >= ST7735_HEIGHT) h = ST7735_HEIGHT - y;

    ST7735_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint32_t total_pixels = w * h;
    uint8_t color_data[2] = {color >> 8, color & 0xFF};
    
    // For large areas, we need to send the color data multiple times
    // This is a simplified approach - in practice, you might want to use a larger buffer
    for (uint32_t i = 0; i < total_pixels; i++)
    {
        ST7735_WriteDataDMA(color_data, 2);
    }
}

// Fill entire screen with color
void ST7735_FillScreen(uint16_t color)
{
    ST7735_FillRectangle(0, 0, ST7735_WIDTH, ST7735_HEIGHT, color);
}

// Set rotation (simplified implementation)
void ST7735_SetRotation(uint8_t rotation)
{
    ST7735_WriteCommand(0x36);
    switch (rotation)
    {
        case 0:
            ST7735_WriteData(0xC8);
            break;
        case 1:
            ST7735_WriteData(0x68);
            break;
        case 2:
            ST7735_WriteData(0x08);
            break;
        case 3:
            ST7735_WriteData(0xA8);
            break;
    }
}

void ST7735_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    ST7735_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                ST7735_WriteDataDMA(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                ST7735_WriteDataDMA(data, sizeof(data));
            }
        }
    }
}

// Write string (simplified implementation - would need font data)
void ST7735_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bg_color)
{
	HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET); // Select device

    while(*str) {
        if(x + font.width >= ST7735_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ST7735_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        ST7735_WriteChar(x, y, *str, font, color, bg_color);
        x += font.width;
        str++;
    }

    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET); // Deselect device
    // This is a placeholder implementation
    // In a real implementation, you would iterate through the string,
    // look up each character in the font data, and draw it pixel by pixel
    // For now, we'll just draw a colored rectangle as a placeholder
//    ST7735_FillRectangle(x, y, strlen(str) * font.width, font.height, color);
}

// Placeholder font definitions (would normally be in separate font files)
/*FontDef Font_7x10 = {NULL, 7, 10, 0x20, 0x7E};
FontDef Font_11x18 = {NULL, 11, 18, 0x20, 0x7E};
FontDef Font_16x26 = {NULL, 16, 26, 0x20, 0x7E};*/

