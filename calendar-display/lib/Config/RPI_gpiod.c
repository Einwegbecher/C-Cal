/*****************************************************************************
* | File        :   RPI_lgpio.c
* | Author      :   Waveshare team (modified for lgpio)
* | Function    :   Drive GPIO using lgpio library
* | Info        :   Read and write gpio
*----------------
* |\tThis version:   V1.0
* | Date        :   2023-11-15
* | Info        :   Modified to use lgpio instead of libgpiod
*
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "RPI_gpiod.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lgpio.h>

int gpiochip;
int gpioline;
int ret;

int GPIOD_Export()
{  
    char buffer[NUM_MAXBUF];
    FILE *fp;

    fp = popen("cat /proc/cpuinfo | grep 'Raspberry Pi 5'", "r");
    if (fp == NULL) {
        GPIOD_Debug("It is not possible to determine the model of the Raspberry PI\n");
        return -1;
    }

    if(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        gpiochip = lgpio_chip_open("/dev/gpiochip4");
        if (gpiochip < 0)
        {
            GPIOD_Debug( "gpiochip4 Export Failed\n");
            return -1;
        }
    }
    else
    {
        gpiochip = lgpio_chip_open("/dev/gpiochip0");
        if (gpiochip < 0)
        {
            GPIOD_Debug( "gpiochip0 Export Failed\n");
            return -1;
        }
    }
    
    pclose(fp);
    
    return 0;
}

int GPIOD_Unexport(int Pin)
{
    if (gpioline >= 0) {
        lgpio_line_release(gpioline);
        gpioline = -1;
    }
    if (gpiochip >= 0) {
        lgpio_chip_close(gpiochip);
        gpiochip = -1;
    }
    return 0;
}

int GPIOD_Unexport_GPIO(int Pin)
{
    if (gpioline >= 0) {
        lgpio_line_release(gpioline);
        gpioline = -1;
    }
    return 0;
}

int GPIOD_Direction(int Pin, int Dir)
{
    gpioline = lgpio_chip_get_line(gpiochip, Pin);
    if (gpioline < 0) {
        GPIOD_Debug("gpiod_chip_get_line Failed\n");
        return -1;
    }

    if (Dir == GPIOD_IN) {
        ret = lgpio_line_request_input(gpioline, "gpio");
        if (ret < 0) {
            GPIOD_Debug("lgpio_line_request_input Failed\n");
            return -1;
        }
    } else {
        ret = lgpio_line_request_output(gpioline, "gpio", 0);
        if (ret < 0) {
            GPIOD_Debug("lgpio_line_request_output Failed\n");
            return -1;
        }
    }
    return 0;
}

int GPIOD_Read(int Pin)
{
    int value = 0;
    
    gpioline = lgpio_chip_get_line(gpiochip, Pin);
    if (gpioline < 0) {
        GPIOD_Debug("gpiod_chip_get_line Failed\n");
        return -1;
    }

    ret = lgpio_line_get_value(gpioline);
    if (ret < 0) {
        GPIOD_Debug("lgpio_line_get_value Failed\n");
        return -1;
    }
    
    value = ret;
    return value;
}

int GPIOD_Write(int Pin, int Value)
{
    gpioline = lgpio_chip_get_line(gpiochip, Pin);
    if (gpioline < 0) {
        GPIOD_Debug("gpiod_chip_get_line Failed\n");
        return -1;
    }

    ret = lgpio_line_set_value(gpioline, Value);
    if (ret < 0) {
        GPIOD_Debug("lgpio_line_set_value Failed\n");
        return -1;
    }
    
    return 0;
}
