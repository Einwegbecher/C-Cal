# CalDAV Integration for C-Cal

## Overview

This directory contains the CalDAV integration code for the C-Cal e-Paper calendar application. The integration allows fetching calendar entries from a CalDAV server and saving them as plaintext files.

## Files Created

### 1. Configuration Files
- **`caldav_config.h`** - Header file defining the CalDAV configuration structure and function prototypes
- **`caldav_config.c`** - Implementation of configuration loading, saving, and management
- **`caldav_config.conf.example`** - Sample configuration file template

### 2. Calendar Entries Files
- **`calendar_entries.h`** - Header file defining calendar entry structure and function prototypes
- **`calendar_entries.c`** - Implementation of calendar entry fetching, parsing, and file saving

### 3. Example Program
- **`examples/caldav_example.c`** - Example program demonstrating how to use the CalDAV integration

## Configuration

### Configuration File Format

The CalDAV configuration file (`caldav_config.conf` by default) uses a simple key=value format:

```ini
# CalDAV server endpoint URL
endpoint=https://caldav.example.com/caldav.php

# Authentication username
username=your_username

# Authentication password
password=your_password

# Name of the calendar to fetch
calendar_name=personal

# Use SSL/TLS (true/false)
use_ssl=true

# Server port
port=443
```

### Supported CalDAV Servers

The integration is designed to work with various CalDAV servers including:
- Nextcloud / ownCloud
- Radicale
- Baikal
- Google Calendar (via CalDAV API)
- Any standard CalDAV-compliant server

## Usage

### Basic Usage

1. **Copy and edit the configuration file:**
   ```bash
   cp caldav_config.conf.example caldav_config.conf
   # Edit caldav_config.conf with your CalDAV server details
   ```

2. **Compile the example program:**
   ```bash
   make clean
   make
   ```

3. **Run the example:**
   ```bash
   ./epd
   ```

### Integration with Existing Code

To integrate CalDAV functionality into your existing C-Cal application:

```c
#include "caldav_config.h"
#include "calendar_entries.h"

int main() {
    CalDAV_Config config;
    Calendar_Entry entries[MAX_CALENDAR_ENTRIES];
    
    // Load configuration
    CALDAV_Config_Init(&config);
    CALDAV_Config_Load(&config, NULL);
    
    // Fetch today's entries
    int count = CALENDAR_FetchTodaysEntries(&config, entries, MAX_CALENDAR_ENTRIES);
    
    // Save to file
    CALENDAR_SaveEntriesToFile(entries, count, NULL);
    
    return 0;
}
```

## Output Files

### Configuration File
- **Default location:** `/etc/ccal_caldav.conf`
- **Purpose:** Stores CalDAV server connection details
- **Format:** Plain text, key=value pairs

### Calendar Entries File
- **Default location:** `/tmp/ccal_today_entries.txt`
- **Purpose:** Stores today's calendar entries as plaintext
- **Format:** Human-readable text with event times and summaries

Example output:
```
Today's Calendar Entries
=========================

08:00 - 09:00 - Team Meeting (Conference Room A)
12:30 - 13:30 - Lunch with Client (Restaurant Downtown)
All Day - Birthday Party (Home)
```

## API Reference

### CalDAV Configuration Functions

```c
// Initialize configuration with defaults
void CALDAV_Config_Init(CalDAV_Config *config);

// Load configuration from file
int CALDAV_Config_Load(CalDAV_Config *config, const char *filepath);

// Save configuration to file
int CALDAV_Config_Save(const CalDAV_Config *config, const char *filepath);

// Generate sample configuration file
int CALDAV_Config_GenerateSample(const char *filepath);
```

### Calendar Entries Functions

```c
// Fetch today's calendar entries from CalDAV server
int CALENDAR_FetchTodaysEntries(const CalDAV_Config *config, 
                                Calendar_Entry *entries, 
                                int max_entries);

// Save calendar entries to a plaintext file
int CALENDAR_SaveEntriesToFile(const Calendar_Entry *entries, 
                               int count, 
                               const char *filepath);

// Format calendar entries as plaintext
int CALENDAR_FormatEntries(const Calendar_Entry *entries, 
                           int count, 
                           char *buffer, 
                           size_t buffer_size);

// Get current date as string (YYYY-MM-DD)
int CALENDAR_GetCurrentDate(char *buffer, size_t buffer_size);

// Parse ISO 8601 datetime string
int CALENDAR_ParseISO8601(const char *datetime, 
                         int *year, int *month, int *day, 
                         int *hour, int *minute);

// Check if an entry is for today
bool CALENDAR_IsEntryToday(const Calendar_Entry *entry, 
                           int year, int month, int day);
```

## Data Structures

### CalDAV_Config
```c
typedef struct {
    char endpoint[256];      // CalDAV server endpoint URL
    char username[128];      // Username for authentication
    char password[128];      // Password for authentication
    char calendar_name[128]; // Name of the calendar to fetch
    bool use_ssl;            // Whether to use SSL/TLS
    uint16_t port;          // Port number
} CalDAV_Config;
```

### Calendar_Entry
```c
typedef struct {
    char summary[256];       // Event title/summary
    char start_time[64];     // Start time as ISO 8601 string
    char end_time[64];       // End time as ISO 8601 string
    char location[128];     // Event location
    bool is_all_day;         // Whether this is an all-day event
} Calendar_Entry;
```

## Implementation Notes

### HTTP Requests

The current implementation includes a simplified HTTP request function that returns mock data for demonstration purposes. In a production environment, you should replace this with a proper HTTP client library such as:

- **libcurl** (recommended)
- **libneon**
- **Custom implementation using sockets**

### XML Parsing

The current implementation includes simplified XML parsing for demonstration. For production use, consider using a proper XML parser library such as:

- **libxml2**
- **expat**
- **Custom SAX parser**

### SSL/TLS Support

For HTTPS connections, you will need to link with an SSL library such as:
- **OpenSSL**
- **GnuTLS**
- **mbed TLS**

### Error Handling

The code includes basic error handling. You may want to enhance it with:
- More detailed error messages
- Logging functionality
- Retry logic for failed requests

## Building

The Makefile has been updated to include the new CalDAV files. Simply run:

```bash
make clean
make
```

The CalDAV files will be compiled and linked with the main application.

## Testing

To test the CalDAV integration:

1. Create a configuration file with your CalDAV server details
2. Run the example program
3. Check the output file for today's calendar entries

## Future Enhancements

Possible future enhancements include:
- Support for recurring events
- Timezone handling
- Caching of calendar data
- Two-way synchronization
- Support for multiple calendars
- Better error handling and logging
- Support for calendar colors/categories
