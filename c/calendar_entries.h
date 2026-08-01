/*****************************************************************************
* | File      :   calendar_entries.h
* | Author    :   C-Cal Project
* | Function  :   Calendar entries management
* | Info      :   Header file for fetching and saving calendar entries
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

#ifndef _CALENDAR_ENTRIES_H_
#define _CALENDAR_ENTRIES_H_

#include "caldav_config.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Calendar Entry Structure
 * Represents a single calendar event
 */
typedef struct {
    char summary[256];       // Event title/summary
    char start_time[64];     // Start time as string (YYYY-MM-DD HH:MM:SS)
    char end_time[64];       // End time as string (YYYY-MM-DD HH:MM:SS)
    char location[128];     // Event location
    bool is_all_day;         // Whether this is an all-day event
} Calendar_Entry;

/**
 * Maximum number of calendar entries to store
 */
#define MAX_CALENDAR_ENTRIES 50

/**
 * Function prototypes
 */

/**
 * @brief Fetch today's calendar entries from CalDAV server
 * @param config Pointer to CalDAV_Config structure
 * @param entries Array of Calendar_Entry to populate
 * @param max_entries Maximum number of entries to fetch
 * @return Number of entries fetched, or -1 on error
 */
int CALENDAR_FetchTodaysEntries(const CalDAV_Config *config, Calendar_Entry *entries, int max_entries);

/**
 * @brief Save calendar entries to a plaintext file
 * @param entries Array of Calendar_Entry
 * @param count Number of entries in the array
 * @param filepath Path to output file (use NULL for default)
 * @return 0 on success, -1 on error
 */
int CALENDAR_SaveEntriesToFile(const Calendar_Entry *entries, int count, const char *filepath);

/**
 * @brief Format calendar entries as plaintext
 * @param entries Array of Calendar_Entry
 * @param count Number of entries in the array
 * @param buffer Buffer to write formatted text to
 * @param buffer_size Size of the buffer
 * @return Number of bytes written, or -1 on error
 */
int CALENDAR_FormatEntries(const Calendar_Entry *entries, int count, char *buffer, size_t buffer_size);

/**
 * @brief Get current date as string (YYYY-MM-DD)
 * @param buffer Buffer to write date string to
 * @param buffer_size Size of the buffer
 * @return 0 on success, -1 on error
 */
int CALENDAR_GetCurrentDate(char *buffer, size_t buffer_size);

/**
 * @brief Parse ISO 8601 datetime string to get time components
 * @param datetime ISO 8601 datetime string (e.g., "20240101T120000Z")
 * @param year Output: year
 * @param month Output: month (1-12)
 * @param day Output: day (1-31)
 * @param hour Output: hour (0-23)
 * @param minute Output: minute (0-59)
 * @return 0 on success, -1 on error
 */
int CALENDAR_ParseISO8601(const char *datetime, int *year, int *month, int *day, int *hour, int *minute);

/**
 * @brief Check if an entry is for today
 * @param entry Pointer to Calendar_Entry
 * @param year Today's year
 * @param month Today's month
 * @param day Today's day
 * @return true if entry is for today, false otherwise
 */
bool CALENDAR_IsEntryToday(const Calendar_Entry *entry, int year, int month, int day);

#endif // _CALENDAR_ENTRIES_H_
