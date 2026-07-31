# Calendar Display System for Raspberry Pi

A C-based application for Raspberry Pi that hosts a lightweight web UI for adding CalDAV or iCal endpoints, fetches today's calendar entries, and renders them on a Waveshare 2.15" G E-Paper display.

## Features

- **Web UI**: Lightweight HTTP server (port 8080) for configuration
- **Calendar Support**: CalDAV (with authentication) and iCal (public URLs)
- **E-Paper Display**: Full color rendering on Waveshare 2.15" G display (black/white/red/yellow)
- **Auto-Refresh**: Configurable refresh interval (default: 15 minutes)
- **Color Coding**: Red for high-priority events, yellow for all-day events
- **Secure Storage**: Configuration saved to `~/.calendar_config.json`

## Hardware Requirements

- **Raspberry Pi**: Any model with GPIO access (Pi 4/5/Zero 2 W recommended)
- **Display**: Waveshare 2.15" G E-Paper Display (250x122 pixels, tri-color)
- **OS**: Raspberry Pi OS (64-bit or 32-bit, headless)

## Wiring Diagram

Connect the Waveshare 2.15" G E-Paper display to your Raspberry Pi as follows:

| E-Paper Pin | Raspberry Pi Pin | GPIO Number | Physical Pin |
|-------------|------------------|-------------|--------------|
| VCC         | 3.3V             | -           | Pin 1        |
| GND         | GND              | -           | Pin 6        |
| DIN         | MOSI             | GPIO 10    | Pin 19       |
| CLK         | SCLK             | GPIO 11    | Pin 23       |
| CS          | CE0              | GPIO 8     | Pin 24       |
| DC          | -                | GPIO 25    | Pin 22       |
| RST         | -                | GPIO 17    | Pin 11       |
| BUSY        | -                | GPIO 24    | Pin 18       |

**Note**: The display uses SPI interface. Make sure SPI is enabled on your Raspberry Pi.

## Installation

### 1. Enable SPI on Raspberry Pi

```bash
sudo raspi-config
```

Navigate to:
- Interface Options -> SPI -> Enable

Reboot your Raspberry Pi.

### 2. Install Dependencies

```bash
# Clone the repository
cd /workspace/Einwegbecher__C-Cal

# Run the installation script
sudo ./calendar-display/install.sh
```

The installation script will:
- Install all required dependencies (gcc, make, libcurl, libmicrohttpd, libgpiod, etc.)
- Create necessary directories
- Build the application
- Create a systemd service

### 3. Manual Build (Optional)

If you prefer to build manually:

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y gcc make libcurl4-openssl-dev libmicrohttpd-dev libgpiod-dev

# Build the application
cd /workspace/Einwegbecher__C-Cal/calendar-display
make clean
make
```

## Usage

### Starting the Application

```bash
# Manual start
./calendar_display

# Or using systemd
sudo systemctl start calendar-display
sudo systemctl enable calendar-display  # Enable auto-start on boot
```

### Accessing the Web Interface

1. Open a web browser on any device on the same network
2. Navigate to: `http://<raspberry-pi-ip>:8080`
3. Add your CalDAV or iCal endpoints
4. Click "Refresh All Calendars" to fetch today's events

### Command Line Options

The application currently doesn't support command-line arguments, but you can modify the source code to change:
- Web server port (default: 8080) - edit `src/main.c`
- Refresh interval (default: 15 minutes) - configure via web UI
- Display settings - edit `src/display.c`

## Configuration

### Adding CalDAV Endpoint

1. Select "CalDAV" as the endpoint type
2. Enter the CalDAV URL (e.g., `https://your-server.com/calendars/user/main`)
3. Enter your username and password
4. Give it a name (optional)
5. Click "Add Endpoint"

### Adding iCal Endpoint

1. Select "iCal" as the endpoint type
2. Enter the public iCal URL (e.g., `https://calendar.google.com/calendar/ical/your-calendar/basic.ics`)
3. Give it a name (optional)
4. Click "Add Endpoint"

### Settings

- **Refresh Interval**: How often to fetch calendar data (in minutes)
- **Timezone**: Your local timezone (e.g., "America/New_York", "UTC")

## Display Layout

The E-Paper display shows:

```
[  Thursday, Jul 31  ]  <- Header (current date)
----------------------
09:00 - 10:00 Meeting   <- Event (time in black, title in red for high priority)
12:00 - 13:00 Lunch    <- Event (standard black text)
15:00 - 16:00 Doctor   <- Event (yellow for medium priority)
----------------------
Sync: 3 events         <- Footer (status message)
```

### Color Coding

| Element | Color | Notes |
|---------|-------|-------|
| Event Time | Black | High contrast on white background |
| High Priority Event | Red | Priority >= 2 or marked as important |
| All-Day Event | Yellow | Events without specific time |
| Medium Priority | Yellow | Priority = 1 |
| Header/Footer | Black | Static text |
| Background | White | Default for E-Paper |

## Testing

### Test with Public iCal URL

1. Get a public iCal URL from Google Calendar or other providers
2. Add it as an iCal endpoint in the web UI
3. Click "Refresh All Calendars"
4. Verify events appear on the display

### Test with CalDAV Server

1. Set up a CalDAV server (Nextcloud, Baïkal, etc.)
2. Create a test calendar with some events
3. Add the CalDAV endpoint with your credentials
4. Click "Refresh All Calendars"
5. Verify events appear on the display

### Verify Color Rendering

1. Create events with different priorities
2. Create all-day events
3. Verify that colors display correctly on the E-Paper

## Troubleshooting

### Display Not Working

1. Check wiring connections
2. Verify SPI is enabled: `ls /dev/spi*`
3. Check if the display is detected: Run the Waveshare test program
4. Ensure you're using the correct GPIO pins

### Web Interface Not Accessible

1. Check if the application is running: `ps aux | grep calendar_display`
2. Check if port 8080 is open: `netstat -tuln | grep 8080`
3. Check firewall settings: `sudo ufw status`
4. Try accessing from the Pi itself: `curl http://localhost:8080`

### Calendar Events Not Showing

1. Check the application logs: `journalctl -u calendar-display -f`
2. Verify the calendar URL is correct and accessible
3. Check credentials for CalDAV endpoints
4. Ensure the calendar has events for today

### Common Error Messages

- **"Failed to initialize DEV module"**: Check wiring and SPI configuration
- **"Failed to start web server"**: Check if port 8080 is available
- **"Failed to fetch CalDAV data"**: Check URL, credentials, and network connectivity
- **"No iCalendar data"**: The URL might not point to a valid .ics file

## Project Structure

```
calendar-display/
├── src/
│   ├── main.c              # Entry point
│   ├── config.c/h          # Configuration management
│   ├── caldav_client.c/h   # CalDAV/iCal fetching and parsing
│   ├── display.c/h         # E-Paper rendering
│   └── web_server.c/h      # HTTP server
├── lib/
│   ├── Config/             # Waveshare hardware config
│   ├── e-Paper/            # E-Paper display drivers
│   ├── GUI/                # Graphics library
│   └── Fonts/              # Font definitions
├── web/
│   └── index.html          # Web UI (embedded in C code)
├── Makefile                # Build configuration
├── install.sh              # Installation script
└── README.md               # This file
```

## Dependencies

- **libcurl**: HTTP requests for CalDAV/iCal
- **libmicrohttpd**: Lightweight web server
- **libgpiod**: GPIO control (alternative to WiringPi)
- **Waveshare E-Paper Library**: Display control (included in lib/)

## Performance Considerations

- The application is designed to be lightweight and run on Raspberry Pi Zero 2 W
- Memory usage is minimized by using fixed-size arrays instead of dynamic allocation where possible
- E-Paper updates use partial refresh to reduce flicker and power consumption
- The display enters sleep mode between updates to save power

## Security Notes

- **Credentials**: Passwords are stored in plaintext in the configuration file. For production use, consider:
  - Using OAuth2 tokens instead of passwords
  - Encrypting the configuration file
  - Restricting file permissions: `chmod 600 ~/.calendar_config.json`
- **Network**: The web server runs on port 8080 by default. Consider:
  - Using HTTPS with a reverse proxy (nginx, Apache)
  - Restricting access with firewall rules
  - Changing the default port

## License

This project uses the Waveshare e-Paper library which is provided under the MIT License. The rest of the code is provided as-is without warranty.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Acknowledgments

- Waveshare for their excellent e-Paper display libraries
- The libmicrohttpd team for the lightweight web server
- The libcurl team for the HTTP client library

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the application logs
3. Verify your hardware connections
4. Check the Waveshare documentation for your specific display model

## Future Enhancements

- [ ] Multi-day view (toggle between today/tomorrow)
- [ ] Battery power support with LiPo battery
- [ ] Touch input with GPIO buttons
- [ ] OTA updates via web UI
- [ ] Better error handling and logging
- [ ] Support for more calendar providers
- [ ] Timezone-aware event filtering
- [ ] Recurring event support
