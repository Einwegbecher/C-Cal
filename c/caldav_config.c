/*****************************************************************************
* | File      :   caldav_config.c
* | Author    :   C-Cal Project
* | Function  :   CalDAV configuration management implementation
* | Info      :   Functions for loading, saving, and managing CalDAV configuration
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

#include "caldav_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Helper function to trim whitespace from a string
static void trim_whitespace(char *str) {
    if (str == NULL) return;
    
    // Trim leading whitespace
    char *start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }
    
    // Trim trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    
    // Move trimmed string to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Helper function to get value from key=value pair
static int parse_key_value(const char *line, const char *key, char *value, size_t value_size) {
    if (line == NULL || key == NULL || value == NULL) return -1;
    
    char *line_copy = strdup(line);
    if (line_copy == NULL) return -1;
    
    char *token = strtok(line_copy, "=");
    if (token == NULL) {
        free(line_copy);
        return -1;
    }
    
    // Trim key
    trim_whitespace(token);
    
    if (strcmp(token, key) != 0) {
        free(line_copy);
        return -1;
    }
    
    // Get value
    token = strtok(NULL, "");
    if (token == NULL) {
        free(line_copy);
        return -1;
    }
    
    trim_whitespace(token);
    strncpy(value, token, value_size - 1);
    value[value_size - 1] = '\0';
    
    free(line_copy);
    return 0;
}

void CALDAV_Config_Init(CalDAV_Config *config) {
    if (config == NULL) return;
    
    memset(config, 0, sizeof(CalDAV_Config));
    config->use_ssl = true;
    config->port = 443;
}

int CALDAV_Config_Load(CalDAV_Config *config, const char *filepath) {
    if (config == NULL) return -1;
    
    const char *config_file = filepath != NULL ? filepath : CALDAV_CONFIG_FILE;
    
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        return -1;
    }
    
    CALDAV_Config_Init(config);
    
    char line[512];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Skip comments and empty lines
        trim_whitespace(line);
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }
        
        // Parse each configuration key
        if (parse_key_value(line, "endpoint", config->endpoint, sizeof(config->endpoint)) == 0) {
            continue;
        }
        if (parse_key_value(line, "username", config->username, sizeof(config->username)) == 0) {
            continue;
        }
        if (parse_key_value(line, "password", config->password, sizeof(config->password)) == 0) {
            continue;
        }
        if (parse_key_value(line, "calendar_name", config->calendar_name, sizeof(config->calendar_name)) == 0) {
            continue;
        }
        
        char bool_str[16];
        if (parse_key_value(line, "use_ssl", bool_str, sizeof(bool_str)) == 0) {
            config->use_ssl = (strcmp(bool_str, "true") == 0 || strcmp(bool_str, "1") == 0);
            continue;
        }
        
        char port_str[16];
        if (parse_key_value(line, "port", port_str, sizeof(port_str)) == 0) {
            config->port = (uint16_t)atoi(port_str);
            continue;
        }
    }
    
    fclose(file);
    return 0;
}

int CALDAV_Config_Save(const CalDAV_Config *config, const char *filepath) {
    if (config == NULL) return -1;
    
    const char *config_file = filepath != NULL ? filepath : CALDAV_CONFIG_FILE;
    
    FILE *file = fopen(config_file, "w");
    if (file == NULL) {
        return -1;
    }
    
    fprintf(file, "# CalDAV Configuration File\n");
    fprintf(file, "# Generated by C-Cal application\n\n");
    
    fprintf(file, "# CalDAV server endpoint URL\n");
    fprintf(file, "endpoint=%s\n\n", config->endpoint);
    
    fprintf(file, "# Authentication username\n");
    fprintf(file, "username=%s\n\n", config->username);
    
    fprintf(file, "# Authentication password\n");
    fprintf(file, "password=%s\n\n", config->password);
    
    fprintf(file, "# Name of the calendar to fetch\n");
    fprintf(file, "calendar_name=%s\n\n", config->calendar_name);
    
    fprintf(file, "# Use SSL/TLS (true/false)\n");
    fprintf(file, "use_ssl=%s\n\n", config->use_ssl ? "true" : "false");
    
    fprintf(file, "# Server port\n");
    fprintf(file, "port=%u\n", config->port);
    
    fclose(file);
    return 0;
}

int CALDAV_Config_GenerateSample(const char *filepath) {
    CalDAV_Config config;
    
    CALDAV_Config_Init(&config);
    
    // Set sample values
    strncpy(config.endpoint, "https://caldav.example.com/caldav.php", sizeof(config.endpoint) - 1);
    strncpy(config.username, "your_username", sizeof(config.username) - 1);
    strncpy(config.password, "your_password", sizeof(config.password) - 1);
    strncpy(config.calendar_name, "personal", sizeof(config.calendar_name) - 1);
    config.use_ssl = true;
    config.port = 443;
    
    return CALDAV_Config_Save(&config, filepath);
}
