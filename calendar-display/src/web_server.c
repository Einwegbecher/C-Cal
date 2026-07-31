/*****************************************************************************
* | File      :   web_server.c
* | Author    :   Calendar Display System
* | Function  :   HTTP web server implementation
* | Info      :   Uses libmicrohttpd for hosting web UI
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/

#include "web_server.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// HTML content for the web UI
const char *get_index_html(void) {
    return 
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "    <title>Calendar Display Configuration</title>\n"
    "    <style>\n"
    "        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n"
    "        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n"
    "        h1 { color: #333; text-align: center; }\n"
    "        .section { margin-bottom: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }\n"
    "        .section h2 { margin-top: 0; color: #444; }\n"
    "        label { display: block; margin-bottom: 5px; font-weight: bold; }\n"
    "        input[type=\"text\"], input[type=\"password\"], input[type=\"number\"] { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }\n"
    "        select { width: 100%; padding: 8px; margin-bottom: 10px; border: 1px solid #ddd; border-radius: 4px; }\n"
    "        button { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }\n"
    "        button:hover { background-color: #45a049; }\n"
    "        button.danger { background-color: #f44336; }\n"
    "        button.danger:hover { background-color: #d32f2f; }\n"
    "        .endpoint-list { margin-top: 10px; }\n"
    "        .endpoint-item { padding: 10px; margin-bottom: 10px; background: #f9f9f9; border-radius: 4px; display: flex; justify-content: space-between; align-items: center; }\n"
    "        .endpoint-info { flex: 1; }\n"
    "        .endpoint-actions { margin-left: 10px; }\n"
    "        .status { padding: 10px; margin-top: 10px; background: #e7f3ff; border-radius: 4px; }\n"
    "        .error { color: #d32f2f; }\n"
    "        .success { color: #388e3c; }\n"
    "        .form-group { margin-bottom: 15px; }\n"
    "        .form-row { display: flex; gap: 10px; }\n"
    "        .form-row .form-group { flex: 1; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <div class=\"container\">\n"
    "        <h1>Calendar Display Configuration</h1>\n"
    "        \n"
    "        <div class=\"section\">\n"
    "            <h2>Add New Endpoint</h2>\n"
    "            <form id=\"addEndpointForm\" onsubmit=\"addEndpoint(); return false;\">\n"
    "                <div class=\"form-group\">\n"
    "                    <label for=\"endpointType\">Endpoint Type:</label>\n"
    "                    <select id=\"endpointType\" required>\n"
    "                        <option value=\"ical\">iCal (Public URL)</option>\n"
    "                        <option value=\"caldav\">CalDAV (with authentication)</option>\n"
    "                    </select>\n"
    "                </div>\n"
    "                <div class=\"form-group\">\n"
    "                    <label for=\"endpointName\">Name:</label>\n"
    "                    <input type=\"text\" id=\"endpointName\" placeholder=\"My Calendar\" required>\n"
    "                </div>\n"
    "                <div class=\"form-group\">\n"
    "                    <label for=\"endpointUrl\">URL:</label>\n"
    "                    <input type=\"text\" id=\"endpointUrl\" placeholder=\"https://example.com/calendar.ics\" required>\n"
    "                </div>\n"
    "                <div class=\"form-row\">\n"
    "                    <div class=\"form-group\">\n"
    "                        <label for=\"endpointUsername\">Username (CalDAV only):</label>\n"
    "                        <input type=\"text\" id=\"endpointUsername\" placeholder=\"username\">\n"
    "                    </div>\n"
    "                    <div class=\"form-group\">\n"
    "                        <label for=\"endpointPassword\">Password (CalDAV only):</label>\n"
    "                        <input type=\"password\" id=\"endpointPassword\" placeholder=\"password\">\n"
    "                    </div>\n"
    "                </div>\n"
    "                <button type=\"submit\">Add Endpoint</button>\n"
    "            </form>\n"
    "        </div>\n"
    "        \n"
    "        <div class=\"section\">\n"
    "            <h2>Configured Endpoints</h2>\n"
    "            <div id=\"endpointList\" class=\"endpoint-list\">\n"
    "                <p>No endpoints configured. Add one above.</p>\n"
    "            </div>\n"
    "            <button onclick=\"refreshAll()\">Refresh All Calendars</button>\n"
    "        </div>\n"
    "        \n"
    "        <div class=\"section\">\n"
    "            <h2>Settings</h2>\n"
    "            <form id=\"settingsForm\" onsubmit=\"saveSettings(); return false;\">\n"
    "                <div class=\"form-row\">\n"
    "                    <div class=\"form-group\">\n"
    "                        <label for=\"refreshInterval\">Refresh Interval (minutes):</label>\n"
    "                        <input type=\"number\" id=\"refreshInterval\" min=\"1\" value=\"15\">\n"
    "                    </div>\n"
    "                    <div class=\"form-group\">\n"
    "                        <label for=\"timezone\">Timezone:</label>\n"
    "                        <input type=\"text\" id=\"timezone\" value=\"UTC\" placeholder=\"UTC or America/New_York\">\n"
    "                    </div>\n"
    "                </div>\n"
    "                <button type=\"submit\">Save Settings</button>\n"
    "            </form>\n"
    "        </div>\n"
    "        \n"
    "        <div class=\"section\">\n"
    "            <h2>Status</h2>\n"
    "            <div id=\"statusMessage\" class=\"status\">\n"
    "                System ready.\n"
    "            </div>\n"
    "        </div>\n"
    "    </div>\n"
    "    \n"
    "    <script>\n"
    "        // Load endpoints on page load\n"
    "        document.addEventListener('DOMContentLoaded', function() {\n"
    "            loadEndpoints();\n"
    "            loadSettings();\n"
    "        });\n"
    "        \n"
    "        function loadEndpoints() {\n"
    "            fetch('/endpoints')\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    const list = document.getElementById('endpointList');\n"
    "                    if (data.endpoints && data.endpoints.length > 0) {\n"
    "                        list.innerHTML = '';\n"
    "                        data.endpoints.forEach((endpoint, index) => {\n"
    "                            const item = document.createElement('div');\n"
    "                            item.className = 'endpoint-item';\n"
    "                            item.innerHTML = `\n"
    "                                <div class=\"endpoint-info\">\n"
    "                                    <strong>${endpoint.name || endpoint.url}</strong><br>\n"
    "                                    <small>${endpoint.type === 'caldav' ? 'CalDAV' : 'iCal'}: ${endpoint.url}</small>\n"
    "                                </div>\n"
    "                                <div class=\"endpoint-actions\">\n"
    "                                    <button onclick=\"removeEndpoint(${index})\" class=\"danger\">Remove</button>\n"
    "                                </div>\n"
    "                            `;\n"
    "                            list.appendChild(item);\n"
    "                        });\n"
    "                    } else {\n"
    "                        list.innerHTML = '<p>No endpoints configured. Add one above.</p>';\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error loading endpoints:', error);\n"
    "                    showStatus('Error loading endpoints', 'error');\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function loadSettings() {\n"
    "            fetch('/config')\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    if (data.refresh_interval) {\n"
    "                        document.getElementById('refreshInterval').value = data.refresh_interval;\n"
    "                    }\n"
    "                    if (data.timezone) {\n"
    "                        document.getElementById('timezone').value = data.timezone;\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error loading settings:', error);\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function addEndpoint() {\n"
    "            const type = document.getElementById('endpointType').value;\n"
    "            const name = document.getElementById('endpointName').value;\n"
    "            const url = document.getElementById('endpointUrl').value;\n"
    "            const username = document.getElementById('endpointUsername').value;\n"
    "            const password = document.getElementById('endpointPassword').value;\n"
    "            \n"
    "            const data = new FormData();\n"
    "            data.append('type', type);\n"
    "            data.append('name', name);\n"
    "            data.append('url', url);\n"
    "            data.append('username', username);\n"
    "            data.append('password', password);\n"
    "            \n"
    "            fetch('/add-endpoint', {\n"
    "                method: 'POST',\n"
    "                body: data\n"
    "            })\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    if (data.success) {\n"
    "                        showStatus('Endpoint added successfully!', 'success');\n"
    "                        loadEndpoints();\n"
    "                        document.getElementById('addEndpointForm').reset();\n"
    "                    } else {\n"
    "                        showStatus('Error: ' + (data.error || 'Unknown error'), 'error');\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error adding endpoint:', error);\n"
    "                    showStatus('Error adding endpoint', 'error');\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function removeEndpoint(index) {\n"
    "            if (!confirm('Are you sure you want to remove this endpoint?')) return;\n"
    "            \n"
    "            const data = new FormData();\n"
    "            data.append('index', index);\n"
    "            \n"
    "            fetch('/remove-endpoint', {\n"
    "                method: 'POST',\n"
    "                body: data\n"
    "            })\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    if (data.success) {\n"
    "                        showStatus('Endpoint removed successfully!', 'success');\n"
    "                        loadEndpoints();\n"
    "                    } else {\n"
    "                        showStatus('Error: ' + (data.error || 'Unknown error'), 'error');\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error removing endpoint:', error);\n"
    "                    showStatus('Error removing endpoint', 'error');\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function refreshAll() {\n"
    "            showStatus('Refreshing calendars...', '');\n"
    "            fetch('/refresh', {\n"
    "                method: 'POST'\n"
    "            })\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    if (data.success) {\n"
    "                        showStatus('Calendars refreshed successfully!', 'success');\n"
    "                    } else {\n"
    "                        showStatus('Error: ' + (data.error || 'Unknown error'), 'error');\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error refreshing:', error);\n"
    "                    showStatus('Error refreshing calendars', 'error');\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function saveSettings() {\n"
    "            const refreshInterval = document.getElementById('refreshInterval').value;\n"
    "            const timezone = document.getElementById('timezone').value;\n"
    "            \n"
    "            const data = new FormData();\n"
    "            data.append('refresh_interval', refreshInterval);\n"
    "            data.append('timezone', timezone);\n"
    "            \n"
    "            fetch('/config', {\n"
    "                method: 'POST',\n"
    "                body: data\n"
    "            })\n"
    "                .then(response => response.json())\n"
    "                .then(data => {\n"
    "                    if (data.success) {\n"
    "                        showStatus('Settings saved successfully!', 'success');\n"
    "                    } else {\n"
    "                        showStatus('Error: ' + (data.error || 'Unknown error'), 'error');\n"
    "                    }\n"
    "                })\n"
    "                .catch(error => {\n"
    "                    console.error('Error saving settings:', error);\n"
    "                    showStatus('Error saving settings', 'error');\n"
    "                });\n"
    "        }\n"
    "        \n"
    "        function showStatus(message, type) {\n"
    "            const status = document.getElementById('statusMessage');\n"
    "            status.textContent = message;\n"
    "            status.className = 'status ' + type;\n"
    "        }\n"
    "    </script>\n"
    "</body>\n"
    "</html>\n";
}

// Send HTML response
void send_html_response(struct MHD_Connection *connection, const char *html) {
    struct MHD_Response *response;
    int ret;
    
    response = MHD_create_response_from_buffer(strlen(html), (void *)html, MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return;
    }
    
    MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    if (ret != MHD_YES) {
        printf("Failed to send HTML response\n");
    }
}

// Send JSON response
void send_json_response(struct MHD_Connection *connection, const char *json) {
    struct MHD_Response *response;
    int ret;
    
    response = MHD_create_response_from_buffer(strlen(json), (void *)json, MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return;
    }
    
    MHD_add_response_header(response, "Content-Type", "application/json; charset=utf-8");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    if (ret != MHD_YES) {
        printf("Failed to send JSON response\n");
    }
}

// Send error response
void send_error_response(struct MHD_Connection *connection, int status_code, const char *message) {
    struct MHD_Response *response;
    int ret;
    char json[512];
    
    snprintf(json, sizeof(json), "{\"success\": false, \"error\": \"%s\"}", message ? message : "Unknown error");
    
    response = MHD_create_response_from_buffer(strlen(json), (void *)json, MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return;
    }
    
    MHD_add_response_header(response, "Content-Type", "application/json; charset=utf-8");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    
    if (ret != MHD_YES) {
        printf("Failed to send error response\n");
    }
}

// Handle GET / (root)
void handle_get_root(struct MHD_Connection *connection, const char *url) {
    (void)url; // Unused parameter
    send_html_response(connection, get_index_html());
}

// Handle GET /endpoints
void handle_get_endpoints(struct MHD_Connection *connection) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    // Build JSON response
    char json[4096];
    char *ptr = json;
    int len = 0;
    
    len += snprintf(ptr + len, sizeof(json) - len, "{\"endpoints\": [");
    
    for (int i = 0; i < state->config->endpoint_count; i++) {
        CalendarEndpoint *ep = &state->config->endpoints[i];
        
        if (i > 0) {
            len += snprintf(ptr + len, sizeof(json) - len, ",");
        }
        
        len += snprintf(ptr + len, sizeof(json) - len, 
                        "{\"type\":\"%s\",\"url\":\"%s\",\"name\":\"%s\",\"enabled\":%s}",
                        ep->type == ENDPOINT_CALDAV ? "caldav" : "ical",
                        ep->url,
                        ep->name[0] ? ep->name : "Unnamed",
                        ep->enabled ? "true" : "false");
    }
    
    len += snprintf(ptr + len, sizeof(json) - len, "]}");
    
    send_json_response(connection, json);
}

// Handle GET /config
void handle_get_config(struct MHD_Connection *connection) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    char json[512];
    snprintf(json, sizeof(json), 
            "{\"refresh_interval\":%d,\"timezone\":\"%s\",\"auto_refresh\":%s}",
            state->config->refresh_interval,
            state->config->timezone,
            state->config->auto_refresh ? "true" : "false");
    
    send_json_response(connection, json);
}

// Handle POST /add-endpoint
void handle_post_add_endpoint(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    // Parse form data (simplified - in production use MHD_get_connection_values)
    // For this example, we'll use a simple approach
    
    char type[32] = {0};
    char name[128] = {0};
    char url[512] = {0};
    char username[256] = {0};
    char password[256] = {0};
    
    // Simple form data parsing
    const char *data = upload_data;
    size_t size = upload_data_size;
    
    // Look for type
    char *type_ptr = strstr((char *)data, "type=");
    if (type_ptr) {
        type_ptr += 5;
        char *end = strchr(type_ptr, '&');
        if (end) {
            strncpy(type, type_ptr, end - type_ptr);
        } else {
            strncpy(type, type_ptr, sizeof(type) - 1);
        }
    }
    
    // Look for name
    char *name_ptr = strstr((char *)data, "name=");
    if (name_ptr) {
        name_ptr += 5;
        char *end = strchr(name_ptr, '&');
        if (end) {
            strncpy(name, name_ptr, end - name_ptr);
        } else {
            strncpy(name, name_ptr, sizeof(name) - 1);
        }
    }
    
    // Look for url
    char *url_ptr = strstr((char *)data, "url=");
    if (url_ptr) {
        url_ptr += 4;
        char *end = strchr(url_ptr, '&');
        if (end) {
            strncpy(url, url_ptr, end - url_ptr);
        } else {
            strncpy(url, url_ptr, sizeof(url) - 1);
        }
    }
    
    // Look for username
    char *username_ptr = strstr((char *)data, "username=");
    if (username_ptr) {
        username_ptr += 9;
        char *end = strchr(username_ptr, '&');
        if (end) {
            strncpy(username, username_ptr, end - username_ptr);
        } else {
            strncpy(username, username_ptr, sizeof(username) - 1);
        }
    }
    
    // Look for password
    char *password_ptr = strstr((char *)data, "password=");
    if (password_ptr) {
        password_ptr += 9;
        char *end = strchr(password_ptr, '&');
        if (end) {
            strncpy(password, password_ptr, end - password_ptr);
        } else {
            strncpy(password, password_ptr, sizeof(password) - 1);
        }
    }
    
    // URL decode the values (simple approach)
    for (int i = 0; type[i]; i++) if (type[i] == '+') type[i] = ' ';
    for (int i = 0; name[i]; i++) if (name[i] == '+') name[i] = ' ';
    for (int i = 0; url[i]; i++) if (url[i] == '+') url[i] = ' ';
    for (int i = 0; username[i]; i++) if (username[i] == '+') username[i] = ' ';
    for (int i = 0; password[i]; i++) if (password[i] == '+') password[i] = ' ';
    
    // Add the endpoint
    EndpointType ep_type = strcmp(type, "caldav") == 0 ? ENDPOINT_CALDAV : ENDPOINT_ICAL;
    
    int result = config_add_endpoint(state->config, ep_type, url, username, password, name);
    
    if (result < 0) {
        send_error_response(connection, MHD_HTTP_BAD_REQUEST, "Failed to add endpoint (max endpoints reached)");
        return;
    }
    
    // Save configuration
    config_save(state->config, DEFAULT_CONFIG_FILE);
    
    char json[256];
    snprintf(json, sizeof(json), "{\"success\": true, \"message\": \"Endpoint added\"}");
    send_json_response(connection, json);
}

// Handle POST /remove-endpoint
void handle_post_remove_endpoint(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    // Parse index from form data
    int index = -1;
    
    const char *data = upload_data;
    char *index_ptr = strstr((char *)data, "index=");
    if (index_ptr) {
        index_ptr += 6;
        index = atoi(index_ptr);
    }
    
    if (index < 0 || index >= state->config->endpoint_count) {
        send_error_response(connection, MHD_HTTP_BAD_REQUEST, "Invalid endpoint index");
        return;
    }
    
    // Remove the endpoint
    config_remove_endpoint(state->config, index);
    
    // Save configuration
    config_save(state->config, DEFAULT_CONFIG_FILE);
    
    char json[256];
    snprintf(json, sizeof(json), "{\"success\": true, \"message\": \"Endpoint removed\"}");
    send_json_response(connection, json);
}

// Handle POST /refresh
void handle_post_refresh(struct MHD_Connection *connection) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    // Trigger a refresh of all calendars
    CalendarResult result;
    CalendarEvent all_events[MAX_EVENTS];
    int total_events = 0;
    
    for (int i = 0; i < state->config->endpoint_count; i++) {
        CalendarEndpoint *ep = &state->config->endpoints[i];
        
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
    if (state->display_state) {
        char status_msg[64];
        snprintf(status_msg, sizeof(status_msg), "Sync: %d events", today_count);
        display_render_calendar(state->display_state, today_events, today_count, status_msg);
    }
    
    char json[512];
    snprintf(json, sizeof(json), "{\"success\": true, \"events_found\": %d, \"message\": \"Calendars refreshed\"}", today_count);
    send_json_response(connection, json);
}

// Handle POST /config
void handle_post_config(struct MHD_Connection *connection, const char *upload_data, size_t upload_data_size) {
    WebServerState *state = (WebServerState *)MHD_get_connection_user_value(connection, NULL);
    
    if (!state || !state->config) {
        send_error_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Server not configured");
        return;
    }
    
    // Parse form data
    const char *data = upload_data;
    
    // Look for refresh_interval
    char *refresh_ptr = strstr((char *)data, "refresh_interval=");
    if (refresh_ptr) {
        refresh_ptr += 17;
        state->config->refresh_interval = atoi(refresh_ptr);
    }
    
    // Look for timezone
    char *tz_ptr = strstr((char *)data, "timezone=");
    if (tz_ptr) {
        tz_ptr += 9;
        char *end = strchr(tz_ptr, '&');
        if (end) {
            strncpy(state->config->timezone, tz_ptr, end - tz_ptr);
        } else {
            strncpy(state->config->timezone, tz_ptr, sizeof(state->config->timezone) - 1);
        }
        state->config->timezone[sizeof(state->config->timezone) - 1] = '\0';
    }
    
    // Save configuration
    config_save(state->config, DEFAULT_CONFIG_FILE);
    
    char json[256];
    snprintf(json, sizeof(json), "{\"success\": true, \"message\": \"Settings saved\"}");
    send_json_response(connection, json);
}

// Request handler callback
static int answer_to_connection(void *cls, struct MHD_Connection *connection, 
                               const char *url, const char *method, 
                               const char *version, const char *upload_data,
                               size_t *upload_data_size, void **con_cls) {
    WebServerState *state = (WebServerState *)cls;
    
    (void)version; // Unused parameter
    
    // Set user value for this connection
    MHD_set_connection_user_value(connection, state, NULL);
    
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/") == 0 || strcmp(url, "/index.html") == 0) {
            handle_get_root(connection, url);
        } else if (strcmp(url, "/endpoints") == 0) {
            handle_get_endpoints(connection);
        } else if (strcmp(url, "/config") == 0) {
            handle_get_config(connection);
        } else {
            // Serve static files from web directory
            // For simplicity, we'll just return 404 for now
            send_error_response(connection, MHD_HTTP_NOT_FOUND, "Not found");
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(url, "/add-endpoint") == 0) {
            handle_post_add_endpoint(connection, upload_data, *upload_data_size);
        } else if (strcmp(url, "/remove-endpoint") == 0) {
            handle_post_remove_endpoint(connection, upload_data, *upload_data_size);
        } else if (strcmp(url, "/refresh") == 0) {
            handle_post_refresh(connection);
        } else if (strcmp(url, "/config") == 0) {
            handle_post_config(connection, upload_data, *upload_data_size);
        } else {
            send_error_response(connection, MHD_HTTP_NOT_FOUND, "Not found");
        }
    } else {
        send_error_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Method not allowed");
    }
    
    return MHD_YES;
}

// Start the web server
int web_server_start(WebServerState *state, AppConfig *config, DisplayState *display, int port) {
    if (!state || !config) return -1;
    
    memset(state, 0, sizeof(WebServerState));
    state->config = config;
    state->display_state = display;
    state->port = port;
    
    // Start MHD daemon
    state->daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNAL_POLL_THREAD, port, NULL, NULL,
                                     (MHD_AcceptPolicyCallback)answer_to_connection, state,
                                     MHD_OPTION_END);
    
    if (!state->daemon) {
        printf("Failed to start web server on port %d\n", port);
        return -1;
    }
    
    state->running = 1;
    printf("Web server started on port %d\n", port);
    
    return 0;
}

// Stop the web server
void web_server_stop(WebServerState *state) {
    if (!state) return;
    
    if (state->daemon) {
        MHD_stop_daemon(state->daemon);
        state->daemon = NULL;
    }
    
    state->running = 0;
    printf("Web server stopped\n");
}
