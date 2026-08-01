#include <stdio.h>
#include <stdlib.h>
#include "c/lib/e-Paper/EPD_2in15g.h"
#include "c/lib/GUI/GUI_Paint.h"
#include "c/lib/Fonts/fonts.h"

int main() {
    printf("Four Rectangles with Text Demo\r\n");

    // Initialize hardware
    if(DEV_Module_Init() != 0) {
        printf("Hardware init failed!\r\n");
        return -1;
    }

    // Initialize E-Paper
    printf("Initializing E-Paper...\r\n");
    EPD_2IN15G_Init();
    EPD_2IN15G_Clear(EPD_2IN15G_WHITE);  // Clear screen to white
    DEV_Delay_ms(2000);

    // Create image buffer
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_2IN15G_WIDTH % 4 == 0) ? (EPD_2IN15G_WIDTH / 4) : (EPD_2IN15G_WIDTH / 4 + 1)) * EPD_2IN15G_HEIGHT;

    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to allocate memory!\r\n");
        return -1;
    }

    // Initialize paint
    Paint_NewImage(BlackImage, EPD_2IN15G_WIDTH, EPD_2IN15G_HEIGHT, 90, EPD_2IN15G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_2IN15G_WHITE);

    // Calculate rectangle dimensions
    UWORD rect_width = EPD_2IN15G_WIDTH / 2;
    UWORD rect_height = EPD_2IN15G_HEIGHT / 2;

    // Draw 4 rectangles with specified colors
    // Top-left: RED
    Paint_DrawRectangle(0, 0, rect_width, rect_height, EPD_2IN15G_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    
    // Bottom-left: YELLOW
    Paint_DrawRectangle(0, rect_height, rect_width, EPD_2IN15G_HEIGHT, EPD_2IN15G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    
    // Top-right: YELLOW
    Paint_DrawRectangle(rect_width, 0, EPD_2IN15G_WIDTH, rect_height, EPD_2IN15G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    
    // Bottom-right: RED
    Paint_DrawRectangle(rect_width, rect_height, EPD_2IN15G_WIDTH, EPD_2IN15G_HEIGHT, EPD_2IN15G_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    // Draw text inside each rectangle
    // Top-left (RED) - black text on red background
    Paint_DrawString_EN(10, 10, "Hello", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_RED);
    Paint_DrawString_EN(10, 30, "World", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_RED);
    
    // Bottom-left (YELLOW) - black text on yellow background
    Paint_DrawString_EN(10, rect_height + 10, "E-Paper", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_YELLOW);
    Paint_DrawString_EN(10, rect_height + 30, "Display", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_YELLOW);
    
    // Top-right (YELLOW) - black text on yellow background
    Paint_DrawString_EN(rect_width + 10, 10, "Four", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_YELLOW);
    Paint_DrawString_EN(rect_width + 10, 30, "Colors", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_YELLOW);
    
    // Bottom-right (RED) - black text on red background
    Paint_DrawString_EN(rect_width + 10, rect_height + 10, "2.15inch", &Font12, EPD_2IN15G_BLACK, EPD_2IN15G_RED);
    Paint_DrawString_EN(rect_width + 10, rect_height + 30, "Test", &Font12, EPD_2IN15G_BLACK, EPD_2IN15G_RED);

    // Display on E-Paper
    printf("Displaying rectangles with text...\r\n");
    EPD_2IN15G_Display(BlackImage);
    
    // Keep display on - do NOT clear
    // Sleep to keep the display active
    printf("Keeping display active...\r\n");
    EPD_2IN15G_Sleep();

    // Cleanup but keep the image on display
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);

    // Power off
    printf("Powering off...\r\n");
    DEV_Module_Exit();

    return 0;
}
