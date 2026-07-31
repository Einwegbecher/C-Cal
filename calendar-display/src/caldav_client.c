/*****************************************************************************
* | File      :   caldav_client.c
* | Author    :   Calendar Display System
* | Function  :   CalDAV and iCal client implementation
* | Info      :   Fetches and parses calendar data
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/

#include "caldav_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <sys/time.h>

// For systems without libical, we'll implement a simple iCalendar parser
// This is a basic parser that handles the most common iCalendar features

// Structure to hold HTTP response data
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for libcurl to write response data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

// Initialize calendar result
void calendar_result_init(CalendarResult *result) {
    memset(result, 0, sizeof(CalendarResult));
    result->event_count = 0;
    result->last_error[0] = '\0';
    result->last_fetch_time = 0;
    result->success = 0;
}

// Free calendar result (if we had dynamically allocated memory)
void calendar_result_free(CalendarResult *result) {
    // Currently events are stored in fixed-size arrays, so nothing to free
    (void)result;
}

// Get today's date in YYYYMMDD format
char *get_today_date_string(void) {
    static char date_str[9];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(date_str, sizeof(date_str), "%Y%m%d", tm_info);
    return date_str;
}

// Simple iCalendar line parser
// iCalendar files use line folding: lines can be split with CRLF followed by space/tab
// This function unfolds the lines
static char *unfold_icalendar_lines(const char *data, size_t *out_size) {
    char *unfolded = malloc(strlen(data) + 1);
    if (!unfolded) return NULL;
    
    char *ptr = unfolded;
    const char *src = data;
    
    while (*src) {
        // Copy until we hit CRLF
        const char *line_start = src;
        while (*src && !(*src == '\r' && *(src + 1) == '\n')) {
            *ptr++ = *src++;
        }
        
        if (*src == '\r' && *(src + 1) == '\n') {
            src += 2; // Skip CRLF
            
            // Check if next character is space or tab (continuation)
            if (*src == ' ' || *src == '\t') {
                src++; // Skip the continuation character
                // Don't add newline, continue on same line
            } else {
                *ptr++ = '\n'; // Add newline for non-continuation
            }
        }
    }
    
    *ptr = '\0';
    *out_size = ptr - unfolded;
    return unfolded;
}

// Parse a single iCalendar component (VEVENT, VTODO, etc.)
static void parse_vevent(const char *component, CalendarEvent *event) {
    const char *ptr = component;
    
    // Initialize event
    memset(event, 0, sizeof(CalendarEvent));
    event->priority = 0; // Default priority
    
    while (*ptr) {
        const char *line_start = ptr;
        
        // Find end of line
        while (*ptr && *ptr != '\n') ptr++;
        
        if (*ptr == '\n') ptr++;
        
        // Skip empty lines
        if (line_start == ptr - 1 && *(ptr - 1) == '\n') continue;
        
        // Parse the line: KEY:VALUE or KEY;PARAMS:VALUE
        const char *colon = strchr(line_start, ':');
        if (!colon) continue;
        
        size_t key_len = colon - line_start;
        const char *value = colon + 1;
        
        // Skip whitespace in key
        while (key_len > 0 && (line_start[key_len - 1] == ' ' || line_start[key_len - 1] == '\t')) {
            key_len--;
        }
        
        // Extract key
        char key[64] = {0};
        if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
        strncpy(key, line_start, key_len);
        key[key_len] = '\0';
        
        // Skip whitespace in value
        while (*value == ' ' || *value == '\t') value++;
        
        // Remove trailing whitespace from value
        const char *value_end = value;
        while (*value_end && *value_end != '\n' && *value_end != '\r') value_end++;
        
        char value_str[1024] = {0};
        size_t value_len = value_end - value;
        if (value_len >= sizeof(value_str)) value_len = sizeof(value_str) - 1;
        strncpy(value_str, value, value_len);
        value_str[value_len] = '\0';
        
        // Handle different properties
        if (strcmp(key, "SUMMARY") == 0) {
            strncpy(event->summary, value_str, sizeof(event->summary) - 1);
        } else if (strcmp(key, "DTSTART") == 0) {
            strncpy(event->start_time, value_str, sizeof(event->start_time) - 1);
            // Check if it's a date (not datetime)
            if (strchr(value_str, 'T') == NULL) {
                event->is_all_day = 1;
            }
        } else if (strcmp(key, "DTEND") == 0) {
            strncpy(event->end_time, value_str, sizeof(event->end_time) - 1);
        } else if (strcmp(key, "LOCATION") == 0) {
            strncpy(event->location, value_str, sizeof(event->location) - 1);
        } else if (strcmp(key, "PRIORITY") == 0) {
            int priority = atoi(value_str);
            if (priority >= 1 && priority <= 5) {
                event->priority = priority;
            }
        } else if (strcmp(key, "CLASS") == 0) {
            if (strcmp(value_str, "CONFIDENTIAL") == 0 || strcmp(value_str, "PRIVATE") == 0) {
                event->priority = 2; // Mark as important
            }
        }
    }
}

// Parse iCalendar data and extract events
int parse_icalendar_data(const char *ical_data, size_t data_size, CalendarResult *result) {
    if (!ical_data || data_size == 0) {
        strncpy(result->last_error, "No iCalendar data", sizeof(result->last_error) - 1);
        return -1;
    }
    
    // Unfold lines
    size_t unfolded_size;
    char *unfolded = unfold_icalendar_lines(ical_data, &unfolded_size);
    if (!unfolded) {
        strncpy(result->last_error, "Failed to unfold iCalendar lines", sizeof(result->last_error) - 1);
        return -1;
    }
    
    // Parse the unfolded data
    char *ptr = unfolded;
    int in_vevent = 0;
    char vevent_buffer[4096] = {0};
    char *vevent_ptr = vevent_buffer;
    size_t vevent_size = 0;
    
    while (*ptr) {
        const char *line_start = ptr;
        
        // Find end of line
        while (*ptr && *ptr != '\n') ptr++;
        
        if (*ptr == '\n') ptr++;
        
        // Check for BEGIN:VEVENT
        if (strncmp(line_start, "BEGIN:VEVENT", 12) == 0) {
            in_vevent = 1;
            vevent_ptr = vevent_buffer;
            vevent_size = 0;
            // Copy the line
            size_t line_len = ptr - line_start;
            if (line_len + vevent_size < sizeof(vevent_buffer)) {
                memcpy(vevent_ptr, line_start, line_len);
                vevent_ptr += line_len;
                vevent_size += line_len;
            }
        } else if (strncmp(line_start, "END:VEVENT", 10) == 0) {
            if (in_vevent) {
                // Copy the line
                size_t line_len = ptr - line_start;
                if (line_len + vevent_size < sizeof(vevent_buffer)) {
                    memcpy(vevent_ptr, line_start, line_len);
                    vevent_ptr += line_len;
                    vevent_size += line_len;
                }
                
                // Parse the VEVENT
                if (result->event_count < MAX_EVENTS) {
                    parse_vevent(vevent_buffer, &result->events[result->event_count]);
                    result->event_count++;
                }
                
                in_vevent = 0;
            }
        } else if (in_vevent) {
            // Copy the line to the VEVENT buffer
            size_t line_len = ptr - line_start;
            if (line_len + vevent_size < sizeof(vevent_buffer)) {
                memcpy(vevent_ptr, line_start, line_len);
                vevent_ptr += line_len;
                vevent_size += line_len;
            }
        }
    }
    
    free(unfolded);
    return 0;
}

// Fetch data from a URL using libcurl
static int fetch_url(const char *url, const char *username, const char *password, 
                     char **output_data, size_t *output_size) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    if (!curl) {
        free(chunk.memory);
        return -1;
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    // Set callback to write response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CalendarDisplay/1.0");
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Set SSL verification (disable for self-signed certs, but not recommended for production)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Set authentication if provided
    if (username && username[0] && password && password[0]) {
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
    }
    
    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        free(chunk.memory);
        curl_easy_cleanup(curl);
        return -1;
    }
    
    // Get the response data
    *output_data = chunk.memory;
    *output_size = chunk.size;
    
    // Cleanup
    curl_easy_cleanup(curl);
    
    return 0;
}

// Fetch events from CalDAV endpoint
int caldav_fetch_events(CalendarEndpoint *endpoint, CalendarResult *result) {
    if (!endpoint || !result) return -1;
    
    calendar_result_init(result);
    
    if (endpoint->type != ENDPOINT_CALDAV) {
        strncpy(result->last_error, "Not a CalDAV endpoint", sizeof(result->last_error) - 1);
        return -1;
    }
    
    // For CalDAV, we need to fetch the calendar data
    // This is a simplified approach - in a full implementation, we would:
    // 1. Send a PROPFIND request to discover the calendar home set
    // 2. Send a REPORT request with a calendar-query to get events for today
    // 
    // For simplicity, we'll assume the URL points directly to a .ics file
    // or the calendar's principal URL
    
    char *data = NULL;
    size_t data_size = 0;
    
    // Try to fetch the URL directly (some CalDAV servers allow direct GET on calendar)
    int fetch_result = fetch_url(endpoint->url, endpoint->username, endpoint->password, 
                                &data, &data_size);
    
    if (fetch_result < 0) {
        strncpy(result->last_error, "Failed to fetch CalDAV data", sizeof(result->last_error) - 1);
        return -1;
    }
    
    // Parse the iCalendar data
    int parse_result = parse_icalendar_data(data, data_size, result);
    free(data);
    
    if (parse_result < 0) {
        return -1;
    }
    
    result->success = 1;
    result->last_fetch_time = time(NULL);
    
    return 0;
}

// Fetch events from iCal URL
int ical_fetch_events(const char *url, CalendarResult *result) {
    if (!url || !result) return -1;
    
    calendar_result_init(result);
    
    char *data = NULL;
    size_t data_size = 0;
    
    // Fetch the iCal file
    int fetch_result = fetch_url(url, NULL, NULL, &data, &data_size);
    
    if (fetch_result < 0) {
        strncpy(result->last_error, "Failed to fetch iCal data", sizeof(result->last_error) - 1);
        return -1;
    }
    
    // Parse the iCalendar data
    int parse_result = parse_icalendar_data(data, data_size, result);
    free(data);
    
    if (parse_result < 0) {
        return -1;
    }
    
    result->success = 1;
    result->last_fetch_time = time(NULL);
    
    return 0;
}

// Filter events for a specific date (YYYYMMDD format)
int filter_events_for_date(CalendarEvent *source_events, int source_count, 
                           CalendarEvent *filtered_events, int max_filtered,
                           const char *date_str) {
    int filtered_count = 0;
    
    for (int i = 0; i < source_count && filtered_count < max_filtered; i++) {
        CalendarEvent *event = &source_events[i];
        
        // For all-day events, check if the date matches
        if (event->is_all_day) {
            // All-day event: check if start_time starts with date_str
            if (strncmp(event->start_time, date_str, 8) == 0) {
                filtered_events[filtered_count++] = *event;
            }
        } else {
            // Timed event: check if it occurs on the given date
            // start_time format: YYYYMMDDTHHMMSSZ or YYYYMMDDTHHMMSS
            if (strncmp(event->start_time, date_str, 8) == 0) {
                filtered_events[filtered_count++] = *event;
            }
            // Also check if event spans multiple days
            else if (event->end_time[0] && strncmp(event->end_time, date_str, 8) == 0) {
                // Event ends on this date, so it's happening today
                filtered_events[filtered_count++] = *event;
            }
        }
    }
    
    return filtered_count;
}
