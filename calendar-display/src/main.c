/*****************************************************************************
* | File      :   main.c
* | Author    :   Calendar Display System
* | Function  :   Main entry point for the calendar display application
* | Info      :   Initializes web server, display, and calendar client
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "config.h"
#include "caldav_client.h"
#include "display.h"
#include "web_server.h"

// Global state
static volatile int running = 1;
static AppConfig app_config;
static DisplayState display_state;
static WebServerState web_server_state;

// Mutex for thread safety
static pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

// Signal handler for graceful shutdown
void handle_signal(int sig) {
    (void)sig; // Unused parameter
    running = 0;
}

// Thread function for auto-refresh
void *refresh_thread(void *arg) {
    (void)arg; // Unused parameter
    
    while (running) {
        // Sleep for the refresh interval
        int interval = 15; // Default to 15 minutes
        
        pthread_mutex_lock(&config_mutex);
        if (app_config.refresh_interval > 0) {
            interval = app_config.refresh_interval;
        }
        pthread_mutex_unlock(&config_mutex);
        
        // Sleep in seconds
        for (int i = 0; i < interval * 60 && running; i++) {
            sleep(1);
        }
        
        if (!running) break;
        
        // Perform refresh
        pthread_mutex_lock(&config_mutex);
        
        CalendarResult result;
        CalendarEvent all_events[MAX_EVENTS];
        int total_events = 0;
        
        for (int i = 0; i < app_config.endpoint_count; i++) {
            CalendarEndpoint *ep = &app_config.endpoints[i];
            
            if (!ep->enabled) continue;
            
            CalendarResult ep_result;
            calendar_result_init(&ep_result);
            
            if (ep->type == ENDPOINT_CALDAV) {
                caldav_fetch_events(ep, &ep_result);
            } else {
                ical_fetch_events(ep->url, &ep_result);
            }
            
            // Copy events
            for (int j = 0; j < ep_result.event_count && total_events < MAX_EVENTS; j++) {
                all_events[total_events++] = ep_result.events[j];
            }
        }
        
        // Filter for today's events
        char today[9];
        strncpy(today, get_today_date_string(), sizeof(today));
        today[sizeof(today) - 1] = '\0';
        
        CalendarEvent today_events[MAX_EVENTS];
        int today_count = filter_events_for_date(all_events, total_events, today_events, MAX_EVENTS, today);
        
        // Update display
        if (display_state.initialized) {
            char status_msg[64];
            snprintf(status_msg, sizeof(status_msg), "Sync: %d events", today_count);
            display_render_calendar(&display_state, today_events, today_count, status_msg);
        }
        
        pthread_mutex_unlock(&config_mutex);
    }
    
    return NULL;
}

// Initialize the application
int init_application(void) {
    // Initialize configuration
    config_init(&app_config);
    
    // Load configuration from file
    if (config_load(&app_config, DEFAULT_CONFIG_FILE) < 0) {
        printf("Warning: Failed to load configuration, using defaults\n");
    } else {
        printf("Configuration loaded from %s\n", DEFAULT_CONFIG_FILE);
    }
    
    // Initialize display
    if (display_init(&display_state) < 0) {
        printf("Error: Failed to initialize display\n");
        return -1;
    }
    printf("Display initialized\n");
    
    // Show initial message
    display_render_calendar(&display_state, NULL, 0, "Initializing...");
    
    // Start web server
    if (web_server_start(&web_server_state, &app_config, &display_state, 8080) < 0) {
        printf("Error: Failed to start web server\n");
        display_cleanup(&display_state);
        return -1;
    }
    
    return 0;
}

// Cleanup the application
void cleanup_application(void) {
    // Stop web server
    web_server_stop(&web_server_state);
    
    // Cleanup display
    display_cleanup(&display_state);
    
    // Save configuration
    config_save(&app_config, DEFAULT_CONFIG_FILE);
    
    printf("Application cleaned up\n");
}

// Main entry point
int main(void) {
    printf("Calendar Display System\n");
    printf("=======================\n\n");
    
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
    
    // Initialize application
    if (init_application() < 0) {
        printf("Failed to initialize application\n");
        return 1;
    }
    
    // Start refresh thread
    pthread_t refresh_tid;
    if (pthread_create(&refresh_tid, NULL, refresh_thread, NULL) != 0) {
        printf("Warning: Failed to create refresh thread\n");
    } else {
        printf("Auto-refresh thread started\n");
    }
    
    // Initial refresh
    printf("Performing initial calendar refresh...\n");
    
    CalendarResult result;
    CalendarEvent all_events[MAX_EVENTS];
    int total_events = 0;
    
    for (int i = 0; i < app_config.endpoint_count; i++) {
        CalendarEndpoint *ep = &app_config.endpoints[i];
        
        if (!ep->enabled) continue;
        
        CalendarResult ep_result;
        calendar_result_init(&ep_result);
        
        if (ep->type == ENDPOINT_CALDAV) {
            printf("Fetching CalDAV calendar: %s\n", ep->name[0] ? ep->name : ep->url);
            caldav_fetch_events(ep, &ep_result);
        } else {
            printf("Fetching iCal calendar: %s\n", ep->name[0] ? ep->name : ep->url);
            ical_fetch_events(ep->url, &ep_result);
        }
        
        if (ep_result.success) {
            printf("  Found %d events\n", ep_result.event_count);
        } else {
            printf("  Error: %s\n", ep_result.last_error);
        }
        
        // Copy events
        for (int j = 0; j < ep_result.event_count && total_events < MAX_EVENTS; j++) {
            all_events[total_events++] = ep_result.events[j];
        }
    }
    
    // Filter for today's events
    char today[9];
    strncpy(today, get_today_date_string(), sizeof(today));
    today[sizeof(today) - 1] = '\0';
    
    CalendarEvent today_events[MAX_EVENTS];
    int today_count = filter_events_for_date(all_events, total_events, today_events, MAX_EVENTS, today);
    
    // Update display
    if (display_state.initialized) {
        char status_msg[64];
        snprintf(status_msg, sizeof(status_msg), "Sync: %d events", today_count);
        display_render_calendar(&display_state, today_events, today_count, status_msg);
    }
    
    printf("\nApplication ready.\n");
    printf("Web UI available at: http://<this-pi-ip>:8080\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    // Main loop - just wait for signals
    while (running) {
        sleep(1);
    }
    
    printf("\nShutting down...\n");
    
    // Wait for refresh thread to finish
    if (refresh_tid) {
        pthread_join(refresh_tid, NULL);
    }
    
    // Cleanup
    cleanup_application();
    
    return 0;
}
