/*****************************************************************************
* | File      :   caldav_client.h
* | Author    :   Calendar Display System
* | Function  :   CalDAV and iCal client for fetching calendar events
* | Info      :   Uses libcurl for HTTP requests and libical for parsing
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/
#ifndef __CALDAV_CLIENT_H_
#define __CALDAV_CLIENT_H_

#include "config.h"
#include <time.h>

// Maximum number of events to fetch
#define MAX_EVENTS 50

// Calendar fetch result
typedef struct {
    CalendarEvent events[MAX_EVENTS];
    int event_count;
    char last_error[512];
    time_t last_fetch_time;
    int success;
} CalendarResult;

// Function prototypes
int caldav_fetch_events(CalendarEndpoint *endpoint, CalendarResult *result);
int ical_fetch_events(const char *url, CalendarResult *result);
void calendar_result_init(CalendarResult *result);
void calendar_result_free(CalendarResult *result);

// Parse iCalendar data and filter for today's events
int parse_icalendar_data(const char *ical_data, size_t data_size, CalendarResult *result);

// Get today's date in YYYYMMDD format for filtering
char *get_today_date_string(void);

// Helper function to extract events for a specific date
int filter_events_for_date(CalendarEvent *source_events, int source_count, 
                           CalendarEvent *filtered_events, int max_filtered,
                           const char *date_str);

#endif // __CALDAV_CLIENT_H_
