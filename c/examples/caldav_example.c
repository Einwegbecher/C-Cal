/*****************************************************************************
* | File      :   caldav_example.c
* | Author    :   C-Cal Project
* | Function  :   Example program demonstrating CalDAV integration
* | Info      :   Shows how to load config, fetch calendar entries, and save to file
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

#include <stdio.h>
#include <stdlib.h>
#include "../caldav_config.h"
#include "../calendar_entries.h"

int main(void) {
    printf("CalDAV Integration Example\n");
    printf("==========================\n\n");
    
    // Step 1: Initialize configuration
    CalDAV_Config config;
    CALDAV_Config_Init(&config);
    
    // Step 2: Try to load configuration from file
    printf("Loading CalDAV configuration...\n");
    if (CALDAV_Config_Load(&config, NULL) == 0) {
        printf("Configuration loaded successfully!\n");
        printf("  Endpoint: %s\n", config.endpoint);
        printf("  Username: %s\n", config.username);
        printf("  Calendar: %s\n", config.calendar_name);
        printf("  Use SSL: %s\n", config.use_ssl ? "Yes" : "No");
        printf("  Port: %u\n\n", config.port);
    } else {
        printf("Configuration file not found. Using defaults.\n");
        printf("  Endpoint: %s\n", config.endpoint);
        printf("  Username: %s\n", config.username);
        printf("  Calendar: %s\n\n", config.calendar_name);
        
        // Generate a sample configuration file
        printf("Generating sample configuration file...\n");
        if (CALDAV_Config_GenerateSample("caldav_config.conf.example") == 0) {
            printf("Sample configuration file generated: caldav_config.conf.example\n");
            printf("Please edit this file with your CalDAV server details.\n\n");
        }
    }
    
    // Step 3: Fetch today's calendar entries
    printf("Fetching today's calendar entries...\n");
    Calendar_Entry entries[MAX_CALENDAR_ENTRIES];
    int entry_count = CALENDAR_FetchTodaysEntries(&config, entries, MAX_CALENDAR_ENTRIES);
    
    if (entry_count < 0) {
        printf("Error fetching calendar entries.\n");
        return 1;
    }
    
    printf("Found %d calendar entries for today.\n\n", entry_count);
    
    // Step 4: Display entries on console
    if (entry_count > 0) {
        printf("Today's Calendar Entries:\n");
        printf("=========================\n");
        
        for (int i = 0; i < entry_count; i++) {
            const Calendar_Entry *entry = &entries[i];
            
            int start_hour, start_min, end_hour, end_min;
            if (CALENDAR_ParseISO8601(entry->start_time, NULL, NULL, NULL, &start_hour, &start_min) == 0 &&
                CALENDAR_ParseISO8601(entry->end_time, NULL, NULL, NULL, &end_hour, &end_min) == 0) {
                if (entry->is_all_day) {
                    printf("All Day - %s", entry->summary);
                } else {
                    printf("%02d:%02d - %02d:%02d - %s", start_hour, start_min, end_hour, end_min, entry->summary);
                }
            } else {
                printf("Unknown Time - %s", entry->summary);
            }
            
            if (entry->location[0] != '\0') {
                printf(" (%s)", entry->location);
            }
            printf("\n");
        }
        printf("\n");
    } else {
        printf("No calendar entries for today.\n\n");
    }
    
    // Step 5: Save entries to file
    printf("Saving calendar entries to file...\n");
    if (CALENDAR_SaveEntriesToFile(entries, entry_count, NULL) == 0) {
        printf("Calendar entries saved to: %s\n", CALENDAR_ENTRIES_FILE);
    } else {
        printf("Error saving calendar entries to file.\n");
    }
    
    // Step 6: Save current configuration (optional)
    printf("\nSaving current configuration...\n");
    if (CALDAV_Config_Save(&config, "caldav_config.conf") == 0) {
        printf("Configuration saved to: caldav_config.conf\n");
    } else {
        printf("Error saving configuration.\n");
    }
    
    printf("\nDone!\n");
    return 0;
}
