/*****************************************************************************
* | File      :   web_server.h
* | Author    :   Calendar Display System
* | Function  :   HTTP web server for configuration
* | Info      :   Uses libmicrohttpd for hosting web UI
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/
#ifndef __WEB_SERVER_H_
#define __WEB_SERVER_H_

#include "config.h"
#include "caldav_client.h"
#include <microhttpd.h>

// Server state
typedef struct {
    struct MHD_Daemon *daemon;
    AppConfig *config;
    int port;
    int running;
    DisplayState *display_state;
} WebServerState;

// Function prototypes
int web_server_start(WebServerState *state, AppConfig *config, DisplayState *display, int port);
void web_server_stop(WebServerState *state);

// Request handlers
static int answer_to_connection(void *cls, struct MHD_Connection *connection, 
                               const char *url, const char *method, 
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls);

// HTML content
const char *get_index_html(void);
const char *get_config_json(AppConfig *config);

// Helper functions
void send_html_response(struct MHD_Connection *connection, const char *html);
void send_json_response(struct MHD_Connection *connection, const char *json);
void send_error_response(struct MHD_Connection *connection, int status_code, const char *message);

// URL handlers
void handle_get_root(struct MHD_Connection *connection, const char *url);
void handle_get_endpoints(struct MHD_Connection *connection);
void handle_post_add_endpoint(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size);
void handle_post_remove_endpoint(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size);
void handle_post_refresh(struct MHD_Connection *connection);
void handle_post_config(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size);

#endif // __WEB_SERVER_H_
