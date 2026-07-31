/*****************************************************************************
* | File      :   config.c
* | Author    :   Calendar Display System
* | Function  :   Configuration management implementation
* | Info      :   JSON-based configuration for calendar endpoints
*----------------
* | This version:   V1.0
* | Date        :   2024
******************************************************************************/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

// JSON parsing helper functions
static void json_escape_string(char *dest, const char *src, size_t dest_size) {
    size_t i, j = 0;
    for (i = 0; src[i] && j < dest_size - 1; i++) {
        switch (src[i]) {
            case '"': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = '"'; break;
            case '\\': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = '\\'; break;
            case '\b': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = 'b'; break;
            case '\f': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = 'f'; break;
            case '\n': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = 'n'; break;
            case '\r': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = 'r'; break;
            case '\t': dest[j++] = '\\'; if (j < dest_size - 1) dest[j++] = 't'; break;
            default:
                if (src[i] >= 32 && src[i] <= 126) {
                    dest[j++] = src[i];
                } else {
                    // Skip non-printable characters
                }
        }
    }
    dest[j] = '\0';
}

static int json_parse_endpoints(AppConfig *config, const char *json_str) {
    // Simple JSON parser - this is a basic implementation
    // For production, consider using a proper JSON library like jansson or cJSON
    
    char *ptr = (char *)json_str;
    char *endptr;
    
    // Skip whitespace
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
    
    // Expect opening brace
    if (*ptr != '{') return -1;
    ptr++;
    
    config->endpoint_count = 0;
    
    while (*ptr) {
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        if (*ptr == '}') {
            ptr++;
            break;
        }
        
        if (*ptr == ',') {
            ptr++;
            continue;
        }
        
        // Expect key
        if (*ptr != '"') return -1;
        ptr++;
        
        char key[256] = {0};
        int i = 0;
        while (*ptr && *ptr != '"' && i < sizeof(key) - 1) {
            key[i++] = *ptr++;
        }
        key[i] = '\0';
        
        if (*ptr != '"') return -1;
        ptr++;
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        if (*ptr != ':') return -1;
        ptr++;
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        if (strcmp(key, "endpoints") == 0) {
            // Parse endpoints array
            if (*ptr != '[') return -1;
            ptr++;
            
            while (*ptr) {
                // Skip whitespace
                while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
                
                if (*ptr == ']') {
                    ptr++;
                    break;
                }
                
                if (*ptr == ',') {
                    ptr++;
                    continue;
                }
                
                if (config->endpoint_count >= MAX_ENDPOINTS) break;
                
                // Expect object
                if (*ptr != '{') return -1;
                ptr++;
                
                CalendarEndpoint *ep = &config->endpoints[config->endpoint_count];
                ep->type = ENDPOINT_ICAL; // default
                ep->url[0] = '\0';
                ep->username[0] = '\0';
                ep->password[0] = '\0';
                ep->name[0] = '\0';
                ep->enabled = 1;
                
                while (*ptr) {
                    // Skip whitespace
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
                    
                    if (*ptr == '}') {
                        ptr++;
                        break;
                    }
                    
                    if (*ptr == ',') {
                        ptr++;
                        continue;
                    }
                    
                    // Expect key
                    if (*ptr != '"') return -1;
                    ptr++;
                    
                    char ep_key[256] = {0};
                    int k = 0;
                    while (*ptr && *ptr != '"' && k < sizeof(ep_key) - 1) {
                        ep_key[k++] = *ptr++;
                    }
                    ep_key[k] = '\0';
                    
                    if (*ptr != '"') return -1;
                    ptr++;
                    
                    // Skip whitespace
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
                    
                    if (*ptr != ':') return -1;
                    ptr++;
                    
                    // Skip whitespace
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
                    
                    if (strcmp(ep_key, "type") == 0) {
                        if (*ptr == '"') {
                            ptr++;
                            char type_str[32] = {0};
                            int t = 0;
                            while (*ptr && *ptr != '"' && t < sizeof(type_str) - 1) {
                                type_str[t++] = *ptr++;
                            }
                            type_str[t] = '\0';
                            if (*ptr != '"') return -1;
                            ptr++;
                            
                            if (strcmp(type_str, "caldav") == 0) {
                                ep->type = ENDPOINT_CALDAV;
                            } else {
                                ep->type = ENDPOINT_ICAL;
                            }
                        }
                    } else if (strcmp(ep_key, "url") == 0) {
                        if (*ptr == '"') {
                            ptr++;
                            int u = 0;
                            while (*ptr && *ptr != '"' && u < sizeof(ep->url) - 1) {
                                ep->url[u++] = *ptr++;
                            }
                            ep->url[u] = '\0';
                            if (*ptr != '"') return -1;
                            ptr++;
                        }
                    } else if (strcmp(ep_key, "username") == 0) {
                        if (*ptr == '"') {
                            ptr++;
                            int u = 0;
                            while (*ptr && *ptr != '"' && u < sizeof(ep->username) - 1) {
                                ep->username[u++] = *ptr++;
                            }
                            ep->username[u] = '\0';
                            if (*ptr != '"') return -1;
                            ptr++;
                        }
                    } else if (strcmp(ep_key, "password") == 0) {
                        if (*ptr == '"') {
                            ptr++;
                            int p = 0;
                            while (*ptr && *ptr != '"' && p < sizeof(ep->password) - 1) {
                                ep->password[p++] = *ptr++;
                            }
                            ep->password[p] = '\0';
                            if (*ptr != '"') return -1;
                            ptr++;
                        }
                    } else if (strcmp(ep_key, "name") == 0) {
                        if (*ptr == '"') {
                            ptr++;
                            int n = 0;
                            while (*ptr && *ptr != '"' && n < sizeof(ep->name) - 1) {
                                ep->name[n++] = *ptr++;
                            }
                            ep->name[n] = '\0';
                            if (*ptr != '"') return -1;
                            ptr++;
                        }
                    } else if (strcmp(ep_key, "enabled") == 0) {
                        if (*ptr == 't' || *ptr == 'T') {
                            ep->enabled = 1;
                            while (*ptr && *ptr != ',' && *ptr != '}') ptr++;
                        } else if (*ptr == 'f' || *ptr == 'F') {
                            ep->enabled = 0;
                            while (*ptr && *ptr != ',' && *ptr != '}') ptr++;
                        } else if (*ptr >= '0' && *ptr <= '9') {
                            ep->enabled = strtol(ptr, &endptr, 10);
                            ptr = endptr;
                        }
                    }
                }
                
                config->endpoint_count++;
            }
        } else if (strcmp(key, "refresh_interval") == 0) {
            config->refresh_interval = strtol(ptr, &endptr, 10);
            ptr = endptr;
        } else if (strcmp(key, "timezone") == 0) {
            if (*ptr == '"') {
                ptr++;
                int t = 0;
                while (*ptr && *ptr != '"' && t < sizeof(config->timezone) - 1) {
                    config->timezone[t++] = *ptr++;
                }
                config->timezone[t] = '\0';
                if (*ptr != '"') return -1;
                ptr++;
            }
        } else if (strcmp(key, "auto_refresh") == 0) {
            if (*ptr == 't' || *ptr == 'T') {
                config->auto_refresh = 1;
                while (*ptr && *ptr != ',' && *ptr != '}') ptr++;
            } else if (*ptr == 'f' || *ptr == 'F') {
                config->auto_refresh = 0;
                while (*ptr && *ptr != ',' && *ptr != '}') ptr++;
            } else if (*ptr >= '0' && *ptr <= '9') {
                config->auto_refresh = strtol(ptr, &endptr, 10);
                ptr = endptr;
            }
        }
    }
    
    return 0;
}

// Initialize configuration with defaults
void config_init(AppConfig *config) {
    memset(config, 0, sizeof(AppConfig));
    config->endpoint_count = 0;
    config->refresh_interval = 15; // 15 minutes
    strncpy(config->timezone, "UTC", sizeof(config->timezone) - 1);
    config->auto_refresh = 1;
}

// Load configuration from JSON file
int config_load(AppConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        // File doesn't exist, initialize with defaults
        config_init(config);
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (size <= 0) {
        fclose(fp);
        config_init(config);
        return 0;
    }
    
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        config_init(config);
        return -1;
    }
    
    size_t bytes_read = fread(buffer, 1, size, fp);
    fclose(fp);
    buffer[bytes_read] = '\0';
    
    config_init(config);
    
    int result = json_parse_endpoints(config, buffer);
    free(buffer);
    
    if (result < 0) {
        // Parse failed, but we have defaults
        return 0;
    }
    
    return 0;
}

// Save configuration to JSON file
int config_save(AppConfig *config, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }
    
    // Ensure directory exists
    char *dir = strdup(filename);
    if (dir) {
        char *last_slash = strrchr(dir, '/');
        if (last_slash) {
            *last_slash = '\0';
            mkdir(dir, 0755);
        }
        free(dir);
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"endpoints\": [\n");
    
    for (int i = 0; i < config->endpoint_count; i++) {
        CalendarEndpoint *ep = &config->endpoints[i];
        char escaped_url[512];
        char escaped_username[256];
        char escaped_password[256];
        char escaped_name[128];
        
        json_escape_string(escaped_url, ep->url, sizeof(escaped_url));
        json_escape_string(escaped_username, ep->username, sizeof(escaped_username));
        json_escape_string(escaped_password, ep->password, sizeof(escaped_password));
        json_escape_string(escaped_name, ep->name, sizeof(escaped_name));
        
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"type\": \"%s\",\n", ep->type == ENDPOINT_CALDAV ? "caldav" : "ical");
        fprintf(fp, "      \"url\": \"%s\",\n", escaped_url);
        fprintf(fp, "      \"username\": \"%s\",\n", escaped_username);
        fprintf(fp, "      \"password\": \"%s\",\n", escaped_password);
        fprintf(fp, "      \"name\": \"%s\",\n", escaped_name);
        fprintf(fp, "      \"enabled\": %s\n", ep->enabled ? "true" : "false");
        fprintf(fp, "    }");
        
        if (i < config->endpoint_count - 1) {
            fprintf(fp, ",\n");
        } else {
            fprintf(fp, "\n");
        }
    }
    
    fprintf(fp, "  ],\n");
    fprintf(fp, "  \"refresh_interval\": %d,\n", config->refresh_interval);
    fprintf(fp, "  \"timezone\": \"%s\",\n", config->timezone);
    fprintf(fp, "  \"auto_refresh\": %s\n", config->auto_refresh ? "true" : "false");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

// Add a new endpoint
int config_add_endpoint(AppConfig *config, EndpointType type, const char *url, 
                        const char *username, const char *password, const char *name) {
    if (config->endpoint_count >= MAX_ENDPOINTS) {
        return -1; // No more space
    }
    
    CalendarEndpoint *ep = &config->endpoints[config->endpoint_count];
    ep->type = type;
    strncpy(ep->url, url ? url : "", sizeof(ep->url) - 1);
    strncpy(ep->username, username ? username : "", sizeof(ep->username) - 1);
    strncpy(ep->password, password ? password : "", sizeof(ep->password) - 1);
    strncpy(ep->name, name ? name : "", sizeof(ep->name) - 1);
    ep->enabled = 1;
    
    config->endpoint_count++;
    return 0;
}

// Remove an endpoint
int config_remove_endpoint(AppConfig *config, int index) {
    if (index < 0 || index >= config->endpoint_count) {
        return -1;
    }
    
    // Shift all endpoints after the removed one
    for (int i = index; i < config->endpoint_count - 1; i++) {
        config->endpoints[i] = config->endpoints[i + 1];
    }
    
    config->endpoint_count--;
    return 0;
}
