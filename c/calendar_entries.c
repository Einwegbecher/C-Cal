/*****************************************************************************
* | File      :   calendar_entries.c
* | Author    :   C-Cal Project
* | Function  :   Calendar entries management implementation
* | Info      :   Functions for fetching and saving calendar entries
*----------------
* | This version:   V1.0
* | Date        :   2024
* |
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

#include "calendar_entries.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

// Helper function to extract value from XML tag
static void extract_xml_value(const char *xml, const char *tag, char *value, size_t value_size) {
    if (xml == NULL || tag == NULL || value == NULL) return;
    
    char *start_tag = strstr(xml, tag);
    if (start_tag == NULL) return;
    
    char *start = strchr(start_tag, '>');
    if (start == NULL) return;
    start++;
    
    char *end = strchr(start, '<');
    if (end == NULL) return;
    
    size_t len = end - start;
    if (len >= value_size) len = value_size - 1;
    
    strncpy(value, start, len);
    value[len] = '\0';
}

// Helper function to URL encode a string
static char *url_encode(const char *str) {
    if (str == NULL) return NULL;
    
    const char *safe_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    
    size_t len = strlen(str);
    char *encoded = malloc(len * 3 + 1); // Worst case: all chars need encoding
    if (encoded == NULL) return NULL;
    
    char *ptr = encoded;
    for (size_t i = 0; i < len; i++) {
        if (strchr(safe_chars, str[i]) != NULL) {
            *ptr++ = str[i];
        } else {
            sprintf(ptr, "%%%02X", (unsigned char)str[i]);
            ptr += 3;
        }
    }
    *ptr = '\0';
    
    return encoded;
}

// Helper function to perform HTTP request (simplified for CalDAV)
// Note: This is a simplified implementation. In production, you would use libcurl
static int http_request(const char *url, const char *username, const char *password,
                        char *response, size_t response_size) {
    // This is a placeholder for actual HTTP request implementation
    // In a real implementation, you would use libcurl or similar
    
    // For now, we'll return a mock response for demonstration
    // In production, replace this with actual HTTP/HTTPS request code
    
    // Mock response - in reality this would fetch from the CalDAV server
    const char *mock_response = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<C:calendar-multiget xmlns:C=\"urn:ietf:params:xml:ns:caldav\">\n"
        "  <D:response xmlns:D=\"DAV:\">\n"
        "    <D:prop>\n"
        "      <C:calendar-data>\n"
        "        BEGIN:VCALENDAR\n"
        "        VERSION:2.0\n"
        "        BEGIN:VEVENT\n"
        "        SUMMARY:Team Meeting\n"
        "        DTSTART:20240101T100000Z\n"
        "        DTEND:20240101T110000Z\n"
        "        LOCATION:Conference Room A\n"
        "        END:VEVENT\n"
        "        BEGIN:VEVENT\n"
        "        SUMMARY:Lunch with Client\n"
        "        DTSTART:20240101T123000Z\n"
        "        DTEND:20240101T133000Z\n"
        "        LOCATION:Restaurant Downtown\n"
        "        END:VEVENT\n"
        "        END:VCALENDAR\n"
        "      </C:calendar-data>\n"
        "    </D:prop>\n"
        "  </D:response>\n"
        "</C:calendar-multiget>";
    
    strncpy(response, mock_response, response_size - 1);
    response[response_size - 1] = '\0';
    
    return 0; // Success
}

int CALENDAR_GetCurrentDate(char *buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size < 11) return -1;
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    if (tm_info == NULL) return -1;
    
    strftime(buffer, buffer_size, "%Y-%m-%d", tm_info);
    return 0;
}

int CALENDAR_ParseISO8601(const char *datetime, int *year, int *month, int *day, int *hour, int *minute) {
    if (datetime == NULL) return -1;
    
    // Handle different ISO 8601 formats
    // Format: YYYYMMDDTHHMMSSZ or YYYYMMDDTHHMMSS or YYYY-MM-DDTHH:MM:SSZ
    
    int y = 0, m = 0, d = 0, h = 0, min = 0;
    
    // Try to parse YYYYMMDDTHHMMSSZ format
    if (strlen(datetime) >= 15) {
        char date_part[9] = {0};
        char time_part[7] = {0};
        
        strncpy(date_part, datetime, 8);
        date_part[8] = '\0';
        
        // Find the 'T' separator
        char *t_pos = strchr(datetime, 'T');
        if (t_pos != NULL) {
            strncpy(time_part, t_pos + 1, 6);
            time_part[6] = '\0';
        } else {
            strncpy(time_part, datetime + 8, 6);
            time_part[6] = '\0';
        }
        
        y = atoi(date_part);
        m = atoi(date_part + 4);
        d = atoi(date_part + 6);
        h = atoi(time_part);
        min = atoi(time_part + 2);
    } else if (strlen(datetime) >= 19) {
        // Try to parse YYYY-MM-DDTHH:MM:SS format
        char *ptr = (char *)datetime;
        
        y = atoi(ptr);
        while (*ptr && *ptr != '-') ptr++;
        if (*ptr == '-') ptr++;
        
        m = atoi(ptr);
        while (*ptr && *ptr != '-') ptr++;
        if (*ptr == '-') ptr++;
        
        d = atoi(ptr);
        while (*ptr && *ptr != 'T' && *ptr != ' ') ptr++;
        if (*ptr == 'T' || *ptr == ' ') ptr++;
        
        h = atoi(ptr);
        while (*ptr && *ptr != ':') ptr++;
        if (*ptr == ':') ptr++;
        
        min = atoi(ptr);
    }
    
    if (year) *year = y;
    if (month) *month = m;
    if (day) *day = d;
    if (hour) *hour = h;
    if (minute) *minute = min;
    
    return 0;
}

bool CALENDAR_IsEntryToday(const Calendar_Entry *entry, int year, int month, int day) {
    if (entry == NULL) return false;
    
    int entry_year, entry_month, entry_day;
    
    // Parse start time
    if (CALENDAR_ParseISO8601(entry->start_time, &entry_year, &entry_month, &entry_day, NULL, NULL) != 0) {
        return false;
    }
    
    return (entry_year == year && entry_month == month && entry_day == day);
}

int CALENDAR_FetchTodaysEntries(const CalDAV_Config *config, Calendar_Entry *entries, int max_entries) {
    if (config == NULL || entries == NULL || max_entries <= 0) return -1;
    
    // Get current date
    char today_date[11];
    if (CALENDAR_GetCurrentDate(today_date, sizeof(today_date)) != 0) {
        return -1;
    }
    
    int year, month, day;
    sscanf(today_date, "%d-%d-%d", &year, &month, &day);
    
    // Build CalDAV query URL
    // This is a simplified version - actual CalDAV query would be more complex
    char url[512];
    snprintf(url, sizeof(url), "%s/%s", config->endpoint, config->calendar_name);
    
    // Add query parameters for today's date
    char *encoded = url_encode(today_date);
    if (encoded) {
        strncat(url, "?start=", sizeof(url) - strlen(url) - 1);
        strncat(url, encoded, sizeof(url) - strlen(url) - 1);
        strncat(url, "&end=", sizeof(url) - strlen(url) - 1);
        strncat(url, encoded, sizeof(url) - strlen(url) - 1);
        free(encoded);
    }
    
    // Perform HTTP request to CalDAV server
    char response[16384];
    if (http_request(url, config->username, config->password, response, sizeof(response)) != 0) {
        return -1;
    }
    
    // Parse the response (simplified parsing for demonstration)
    // In a real implementation, you would use a proper XML parser
    
    int entry_count = 0;
    char *ptr = response;
    
    // Mock parsing - in reality, parse the iCalendar data from the response
    // For now, we'll create some mock entries
    
    // Entry 1: Team Meeting
    if (entry_count < max_entries) {
        strncpy(entries[entry_count].summary, "Team Meeting", sizeof(entries[entry_count].summary) - 1);
        strncpy(entries[entry_count].start_time, "20240101T100000Z", sizeof(entries[entry_count].start_time) - 1);
        strncpy(entries[entry_count].end_time, "20240101T110000Z", sizeof(entries[entry_count].end_time) - 1);
        strncpy(entries[entry_count].location, "Conference Room A", sizeof(entries[entry_count].location) - 1);
        entries[entry_count].is_all_day = false;
        
        if (CALENDAR_IsEntryToday(&entries[entry_count], year, month, day)) {
            entry_count++;
        }
    }
    
    // Entry 2: Lunch with Client
    if (entry_count < max_entries) {
        strncpy(entries[entry_count].summary, "Lunch with Client", sizeof(entries[entry_count].summary) - 1);
        strncpy(entries[entry_count].start_time, "20240101T123000Z", sizeof(entries[entry_count].start_time) - 1);
        strncpy(entries[entry_count].end_time, "20240101T133000Z", sizeof(entries[entry_count].end_time) - 1);
        strncpy(entries[entry_count].location, "Restaurant Downtown", sizeof(entries[entry_count].location) - 1);
        entries[entry_count].is_all_day = false;
        
        if (CALENDAR_IsEntryToday(&entries[entry_count], year, month, day)) {
            entry_count++;
        }
    }
    
    // Entry 3: All-day event
    if (entry_count < max_entries) {
        strncpy(entries[entry_count].summary, "Birthday Party", sizeof(entries[entry_count].summary) - 1);
        strncpy(entries[entry_count].start_time, "20240101T000000Z", sizeof(entries[entry_count].start_time) - 1);
        strncpy(entries[entry_count].end_time, "20240102T000000Z", sizeof(entries[entry_count].end_time) - 1);
        strncpy(entries[entry_count].location, "Home", sizeof(entries[entry_count].location) - 1);
        entries[entry_count].is_all_day = true;
        
        if (CALENDAR_IsEntryToday(&entries[entry_count], year, month, day)) {
            entry_count++;
        }
    }
    
    return entry_count;
}

int CALENDAR_FormatEntries(const Calendar_Entry *entries, int count, char *buffer, size_t buffer_size) {
    if (entries == NULL || buffer == NULL || buffer_size == 0) return -1;
    
    char *ptr = buffer;
    size_t remaining = buffer_size;
    
    // Write header
    int written = snprintf(ptr, remaining, "Today's Calendar Entries\n");
    if (written < 0) return -1;
    ptr += written;
    remaining -= written;
    
    written = snprintf(ptr, remaining, "=========================\n\n");
    if (written < 0) return -1;
    ptr += written;
    remaining -= written;
    
    if (count == 0) {
        written = snprintf(ptr, remaining, "No entries for today.\n");
        if (written < 0) return -1;
        ptr += written;
        remaining -= written;
    } else {
        for (int i = 0; i < count; i++) {
            const Calendar_Entry *entry = &entries[i];
            
            // Format: Time - Summary (Location)
            char time_str[64];
            int start_hour, start_min, end_hour, end_min;
            
            if (CALENDAR_ParseISO8601(entry->start_time, NULL, NULL, NULL, &start_hour, &start_min) == 0 &&
                CALENDAR_ParseISO8601(entry->end_time, NULL, NULL, NULL, &end_hour, &end_min) == 0) {
                if (entry->is_all_day) {
                    snprintf(time_str, sizeof(time_str), "All Day");
                } else {
                    snprintf(time_str, sizeof(time_str), "%02d:%02d - %02d:%02d", 
                            start_hour, start_min, end_hour, end_min);
                }
            } else {
                snprintf(time_str, sizeof(time_str), "Unknown Time");
            }
            
            if (entry->location[0] != '\0') {
                written = snprintf(ptr, remaining, "%s - %s (%s)\n", 
                                 time_str, entry->summary, entry->location);
            } else {
                written = snprintf(ptr, remaining, "%s - %s\n", 
                                 time_str, entry->summary);
            }
            
            if (written < 0) return -1;
            ptr += written;
            remaining -= written;
        }
    }
    
    return (int)(ptr - buffer);
}

int CALENDAR_SaveEntriesToFile(const Calendar_Entry *entries, int count, const char *filepath) {
    if (entries == NULL || count < 0) return -1;
    
    const char *output_file = filepath != NULL ? filepath : CALENDAR_ENTRIES_FILE;
    
    // Format entries
    char buffer[8192];
    int bytes_written = CALENDAR_FormatEntries(entries, count, buffer, sizeof(buffer));
    if (bytes_written < 0) {
        return -1;
    }
    
    // Write to file
    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        return -1;
    }
    
    fwrite(buffer, 1, bytes_written, file);
    fclose(file);
    
    return 0;
}
