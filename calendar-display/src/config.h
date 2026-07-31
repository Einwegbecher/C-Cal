/*****************************************************************************
* | File      :   config.h
* | Author    :   Calendar Display System
* | Function  :   Configuration management for calendar endpoints
* | Info      :   Handles saving/loading CalDAV and iCal endpoints
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <stdint.h>
#include <time.h>

// Maximum number of endpoints
#define MAX_ENDPOINTS 10

// Endpoint types
typedef enum {
    ENDPOINT_CALDAV,
    ENDPOINT_ICAL
} EndpointType;

// Calendar endpoint configuration
typedef struct {
    EndpointType type;
    char url[512];
    char username[256];
    char password[256];
    char name[128];
    uint8_t enabled;
} CalendarEndpoint;

// Application configuration
typedef struct {
    CalendarEndpoint endpoints[MAX_ENDPOINTS];
    int endpoint_count;
    int refresh_interval; // in minutes
    char timezone[64];
    uint8_t auto_refresh;
} AppConfig;

// Event structure for calendar entries
typedef struct {
    char summary[256];
    char start_time[64];
    char end_time[64];
    char location[256];
    uint8_t is_all_day;
    uint8_t priority; // 0=normal, 1=high (red), 2=medium (yellow)
} CalendarEvent;

// Function prototypes
int config_load(AppConfig *config, const char *filename);
int config_save(AppConfig *config, const char *filename);
void config_init(AppConfig *config);
int config_add_endpoint(AppConfig *config, EndpointType type, const char *url, 
                        const char *username, const char *password, const char *name);
int config_remove_endpoint(AppConfig *config, int index);

// Default config file path
#define DEFAULT_CONFIG_FILE "/home/pi/.calendar_config.json"

#endif // __CONFIG_H_
