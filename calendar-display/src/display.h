/*****************************************************************************
* | File      :   display.h
* | Author    :   Calendar Display System
* | Function  :   E-Paper display rendering for calendar events
* | Info      :   Uses Waveshare 2.15" G E-Paper display
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/
#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "config.h"
#include "caldav_client.h"
#include "DEV_Config.h"
#include "EPD_2in15g.h"
#include "GUI_Paint.h"

// Display dimensions for 2.15" G E-Paper
#define DISPLAY_WIDTH EPD_2IN15G_WIDTH
#define DISPLAY_HEIGHT EPD_2IN15G_HEIGHT

// Color definitions (matching EPD_2in15g.h)
#define COLOR_BLACK EPD_2IN15G_BLACK
#define COLOR_WHITE EPD_2IN15G_WHITE
#define COLOR_RED EPD_2IN15G_RED
#define COLOR_YELLOW EPD_2IN15G_YELLOW

// Display state
typedef struct {
    UBYTE *image_buffer;
    PAINT paint;
    int initialized;
    int needs_refresh;
} DisplayState;

// Function prototypes
int display_init(DisplayState *state);
void display_cleanup(DisplayState *state);
int display_render_calendar(DisplayState *state, CalendarEvent *events, int event_count, 
                            const char *status_message);
int display_render_error(DisplayState *state, const char *error_message);
int display_clear(DisplayState *state);
int display_sleep(DisplayState *state);
int display_wakeup(DisplayState *state);

// Helper functions
void draw_header(PAINT *paint, const char *date_str);
void draw_event_list(PAINT *paint, CalendarEvent *events, int event_count, int y_start);
void draw_footer(PAINT *paint, const char *status_message, int y_pos);
void draw_event(PAINT *paint, CalendarEvent *event, int y_pos, int width);

// Format time strings for display
char *format_event_time(const char *start_time, const char *end_time, char *buffer, size_t buffer_size);
char *format_date_header(char *buffer, size_t buffer_size);

#endif // __DISPLAY_H_
