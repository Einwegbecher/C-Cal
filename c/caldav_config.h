/*****************************************************************************
* | File      :   caldav_config.h
* | Author    :   C-Cal Project
* | Function  :   CalDAV configuration management
* | Info      :   Header file for CalDAV endpoint configuration
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

#ifndef _CALDAV_CONFIG_H_
#define _CALDAV_CONFIG_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * CalDAV Configuration Structure
 * Stores the CalDAV endpoint and authentication details
 */
typedef struct {
    char endpoint[256];      // CalDAV server endpoint URL
    char username[128];      // Username for authentication
    char password[128];      // Password for authentication
    char calendar_name[128]; // Name of the calendar to fetch
    bool use_ssl;            // Whether to use SSL/TLS
    uint16_t port;          // Port number (default: 443 for SSL, 80 for HTTP)
} CalDAV_Config;

/**
 * Default configuration file path
 */
#define CALDAV_CONFIG_FILE "/etc/ccal_caldav.conf"

/**
 * Default calendar entries output file path
 */
#define CALENDAR_ENTRIES_FILE "/tmp/ccal_today_entries.txt"

/**
 * Function prototypes
 */

/**
 * @brief Initialize CalDAV configuration with defaults
 * @param config Pointer to CalDAV_Config structure
 */
void CALDAV_Config_Init(CalDAV_Config *config);

/**
 * @brief Load CalDAV configuration from file
 * @param config Pointer to CalDAV_Config structure to populate
 * @param filepath Path to configuration file (use NULL for default)
 * @return 0 on success, -1 on error
 */
int CALDAV_Config_Load(CalDAV_Config *config, const char *filepath);

/**
 * @brief Save CalDAV configuration to file
 * @param config Pointer to CalDAV_Config structure
 * @param filepath Path to configuration file (use NULL for default)
 * @return 0 on success, -1 on error
 */
int CALDAV_Config_Save(const CalDAV_Config *config, const char *filepath);

/**
 * @brief Generate a sample configuration file
 * @param filepath Path to write the sample config file
 * @return 0 on success, -1 on error
 */
int CALDAV_Config_GenerateSample(const char *filepath);

#endif // _CALDAV_CONFIG_H_
