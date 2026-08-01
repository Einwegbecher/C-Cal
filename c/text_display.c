#include <stdio.h>
#include <stdlib.h>
#include "EPD_2in15g.h"
#include "GUI_Paint.h"
#include "Fonts/fonts.h"

int main() {
    printf("E-Paper Text Display Demo\r\n");

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
    Paint_SetScale(4);  // Scale for better visibility
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_2IN15G_WHITE);

    // Draw only 4 lines of text
    Paint_DrawString_EN(10, 10, "Hello World", &Font16, EPD_2IN15G_BLACK, EPD_2IN15G_WHITE);
    Paint_DrawString_EN(10, 30, "This is line 2", &Font12, EPD_2IN15G_BLACK, EPD_2IN15G_WHITE);
    Paint_DrawString_EN(10, 50, "This is line 3", &Font12, EPD_2IN15G_BLACK, EPD_2IN15G_WHITE);
    Paint_DrawString_EN(10, 70, "This is line 4", &Font12, EPD_2IN15G_BLACK, EPD_2IN15G_WHITE);

    // Display on E-Paper
    printf("Displaying text...\r\n");
    EPD_2IN15G_Display(BlackImage);
    DEV_Delay_ms(5000);  // Display for 5 seconds

    // Cleanup - no longer clearing the screen
    printf("Goto Sleep...\r\n");
    EPD_2IN15G_Sleep();  // Put display to sleep

    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);

    // Power off
    printf("Powering off...\r\n");
    DEV_Module_Exit();

    return 0;
}
