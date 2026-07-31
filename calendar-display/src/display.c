/*****************************************************************************
* | File      :   display.c
* | Author    :   Calendar Display System
* | Function  :   E-Paper display rendering implementation
* | Info      :   Renders calendar events on Waveshare 2.15" G E-Paper
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/

#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Initialize the display
int display_init(DisplayState *state) {
    if (!state) return -1;
    
    memset(state, 0, sizeof(DisplayState));
    
    // Initialize the device
    if (DEV_Module_Init() != 0) {
        printf("Failed to initialize DEV module\n");
        return -1;
    }
    
    // Initialize the E-Paper display
    printf("Initializing E-Paper display...\n");
    EPD_2IN15G_Init();
    
    // Calculate image buffer size
    UWORD image_size = ((DISPLAY_WIDTH % 4 == 0) ? (DISPLAY_WIDTH / 4) : (DISPLAY_WIDTH / 4 + 1)) * DISPLAY_HEIGHT;
    
    // Allocate image buffer
    state->image_buffer = (UBYTE *)malloc(image_size);
    if (!state->image_buffer) {
        printf("Failed to allocate image buffer\n");
        DEV_Module_Exit();
        return -1;
    }
    
    // Initialize paint structure
    Paint_NewImage(state->image_buffer, DISPLAY_WIDTH, DISPLAY_HEIGHT, 90, COLOR_WHITE);
    Paint_SelectImage(state->image_buffer);
    Paint_SetScale(4);
    
    state->initialized = 1;
    state->needs_refresh = 1;
    
    // Clear the display
    display_clear(state);
    
    return 0;
}

// Cleanup the display
void display_cleanup(DisplayState *state) {
    if (!state) return;
    
    if (state->image_buffer) {
        free(state->image_buffer);
        state->image_buffer = NULL;
    }
    
    // Put display to sleep
    display_sleep(state);
    
    // Exit the module
    DEV_Module_Exit();
    
    state->initialized = 0;
}

// Clear the display
int display_clear(DisplayState *state) {
    if (!state || !state->initialized) return -1;
    
    Paint_SelectImage(state->image_buffer);
    Paint_Clear(COLOR_WHITE);
    
    EPD_2IN15G_Display(state->image_buffer);
    
    return 0;
}

// Put display to sleep
int display_sleep(DisplayState *state) {
    if (!state || !state->initialized) return -1;
    
    EPD_2IN15G_Sleep();
    return 0;
}

// Wake up the display
int display_wakeup(DisplayState *state) {
    if (!state || !state->initialized) return -1;
    
    EPD_2IN15G_Init();
    return 0;
}

// Format date header (e.g., "Thursday, Jul 31")
char *format_date_header(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(buffer, buffer_size, "%A, %b %d", tm_info);
    return buffer;
}

// Format event time string
char *format_event_time(const char *start_time, const char *end_time, char *buffer, size_t buffer_size) {
    if (!start_time || !buffer || buffer_size == 0) {
        if (buffer && buffer_size > 0) buffer[0] = '\0';
        return buffer;
    }
    
    // Parse start time
    // Format: YYYYMMDDTHHMMSSZ or YYYYMMDDTHHMMSS
    int start_hour = 0, start_min = 0;
    int end_hour = 0, end_min = 0;
    
    if (strlen(start_time) >= 11) {
        // Extract hours and minutes from start time
        char hour_str[3] = {start_time[9], start_time[10], '\0'};
        char min_str[3] = {start_time[11], start_time[12], '\0'};
        start_hour = atoi(hour_str);
        start_min = atoi(min_str);
    }
    
    if (end_time && strlen(end_time) >= 11) {
        char hour_str[3] = {end_time[9], end_time[10], '\0'};
        char min_str[3] = {end_time[11], end_time[12], '\0'};
        end_hour = atoi(hour_str);
        end_min = atoi(min_str);
    }
    
    if (end_time && (end_hour > 0 || end_min > 0)) {
        // Format as "HH:MM - HH:MM"
        snprintf(buffer, buffer_size, "%02d:%02d - %02d:%02d", 
                start_hour, start_min, end_hour, end_min);
    } else {
        // All-day event or no end time
        snprintf(buffer, buffer_size, "All day");
    }
    
    return buffer;
}

// Draw the header with current date
void draw_header(PAINT *paint, const char *date_str) {
    Paint_SelectImage(paint->Image);
    
    // Draw a line at the top
    Paint_DrawLine(0, 0, DISPLAY_WIDTH - 1, 0, COLOR_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 1, DISPLAY_WIDTH - 1, 1, COLOR_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    
    // Draw date string centered
    int text_width = Font16.Width * strlen(date_str);
    int x_pos = (DISPLAY_WIDTH - text_width) / 2;
    if (x_pos < 0) x_pos = 0;
    
    Paint_DrawString_EN(x_pos, 4, date_str, &Font16, COLOR_BLACK, COLOR_WHITE);
    
    // Draw a separator line
    Paint_DrawLine(0, 22, DISPLAY_WIDTH - 1, 22, COLOR_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

// Draw a single event
void draw_event(PAINT *paint, CalendarEvent *event, int y_pos, int width) {
    char time_buffer[64];
    char display_text[256];
    
    // Format time
    format_event_time(event->start_time, event->end_time, time_buffer, sizeof(time_buffer));
    
    // Determine color based on priority
    UWORD text_color = COLOR_BLACK;
    UWORD bg_color = COLOR_WHITE;
    
    if (event->priority >= 2) {
        text_color = COLOR_RED;
    } else if (event->priority == 1) {
        text_color = COLOR_YELLOW;
    } else if (event->is_all_day) {
        text_color = COLOR_YELLOW;
    }
    
    // Draw time (left-aligned)
    Paint_DrawString_EN(2, y_pos, time_buffer, &Font12, COLOR_BLACK, COLOR_WHITE);
    
    // Draw event title (after time, with color)
    int time_width = Font12.Width * strlen(time_buffer);
    int text_x = 2 + time_width + 4; // 4 pixels padding
    
    // Truncate title if too long
    strncpy(display_text, event->summary, sizeof(display_text) - 1);
    display_text[sizeof(display_text) - 1] = '\0';
    
    // Calculate available width for text
    int available_width = width - text_x - 2;
    int max_chars = available_width / Font12.Width;
    if (max_chars > 0 && strlen(display_text) > (size_t)max_chars) {
        display_text[max_chars - 1] = '.';
        display_text[max_chars] = '.';
        display_text[max_chars + 1] = '.';
        display_text[max_chars + 2] = '\0';
    }
    
    Paint_DrawString_EN(text_x, y_pos, display_text, &Font12, text_color, COLOR_WHITE);
}

// Draw the event list
void draw_event_list(PAINT *paint, CalendarEvent *events, int event_count, int y_start) {
    int y_pos = y_start;
    int line_height = Font12.Height + 2; // 2 pixels spacing
    
    for (int i = 0; i < event_count; i++) {
        if (y_pos + line_height > DISPLAY_HEIGHT - 20) {
            // Not enough space for more events
            break;
        }
        
        draw_event(paint, &events[i], y_pos, DISPLAY_WIDTH);
        y_pos += line_height;
    }
}

// Draw the footer with status message
void draw_footer(PAINT *paint, const char *status_message, int y_pos) {
    Paint_SelectImage(paint->Image);
    
    // Draw a separator line
    Paint_DrawLine(0, y_pos - 2, DISPLAY_WIDTH - 1, y_pos - 2, COLOR_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    
    // Draw status message
    if (status_message && status_message[0]) {
        Paint_DrawString_EN(2, y_pos, status_message, &Font12, COLOR_BLACK, COLOR_WHITE);
    } else {
        // Default message
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "Last sync: %H:%M", tm_info);
        Paint_DrawString_EN(2, y_pos, time_str, &Font12, COLOR_BLACK, COLOR_WHITE);
    }
}

// Render calendar events on the display
int display_render_calendar(DisplayState *state, CalendarEvent *events, int event_count, 
                            const char *status_message) {
    if (!state || !state->initialized) return -1;
    
    Paint_SelectImage(state->image_buffer);
    Paint_Clear(COLOR_WHITE);
    
    // Format date header
    char date_header[64];
    format_date_header(date_header, sizeof(date_header));
    
    // Draw header
    draw_header(&state->paint, date_header);
    
    // Draw event list
    int event_list_y = 24; // Start below header
    
    if (event_count == 0) {
        // No events message
        Paint_DrawString_EN(2, event_list_y, "No events today", &Font16, COLOR_BLACK, COLOR_WHITE);
    } else {
        draw_event_list(&state->paint, events, event_count, event_list_y);
    }
    
    // Draw footer
    int footer_y = DISPLAY_HEIGHT - Font12.Height - 4;
    draw_footer(&state->paint, status_message, footer_y);
    
    // Display the image
    EPD_2IN15G_Display(state->image_buffer);
    
    return 0;
}

// Render error message on the display
int display_render_error(DisplayState *state, const char *error_message) {
    if (!state || !state->initialized) return -1;
    
    Paint_SelectImage(state->image_buffer);
    Paint_Clear(COLOR_WHITE);
    
    // Draw error header
    Paint_DrawString_EN(2, 4, "Error", &Font16, COLOR_RED, COLOR_WHITE);
    Paint_DrawLine(0, 22, DISPLAY_WIDTH - 1, 22, COLOR_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    
    // Draw error message (split into multiple lines if needed)
    int y_pos = 26;
    int line_height = Font12.Height + 2;
    
    if (error_message && error_message[0]) {
        char line1[64] = {0};
        char line2[64] = {0};
        char line3[64] = {0};
        
        // Split message into lines
        strncpy(line1, error_message, sizeof(line1) - 1);
        
        // Find first space after 20 characters for line break
        if (strlen(error_message) > 20) {
            char *space_ptr = strchr(error_message + 20, ' ');
            if (space_ptr) {
                size_t len = space_ptr - error_message;
                strncpy(line1, error_message, len);
                line1[len] = '\0';
                strncpy(line2, space_ptr + 1, sizeof(line2) - 1);
            } else {
                strncpy(line1, error_message, 20);
                line1[20] = '\0';
                strncpy(line2, error_message + 20, sizeof(line2) - 1);
            }
        }
        
        Paint_DrawString_EN(2, y_pos, line1, &Font12, COLOR_BLACK, COLOR_WHITE);
        y_pos += line_height;
        
        if (line2[0]) {
            Paint_DrawString_EN(2, y_pos, line2, &Font12, COLOR_BLACK, COLOR_WHITE);
            y_pos += line_height;
        }
        
        if (line3[0]) {
            Paint_DrawString_EN(2, y_pos, line3, &Font12, COLOR_BLACK, COLOR_WHITE);
        }
    } else {
        Paint_DrawString_EN(2, y_pos, "Unknown error", &Font12, COLOR_BLACK, COLOR_WHITE);
    }
    
    // Display the image
    EPD_2IN15G_Display(state->image_buffer);
    
    return 0;
}
