#!/bin/bash

# Calendar Display System Installation Script
# This script installs all dependencies and sets up the application

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}This script must be run as root (use sudo).${NC}"
    exit 1
fi

# Check if on Raspberry Pi
if ! grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null && \
   ! grep -q "Raspbian" /etc/os-release 2>/dev/null && \
   ! grep -q "Debian" /etc/os-release 2>/dev/null; then
    echo -e "${YELLOW}Warning: This script is designed for Raspberry Pi. Continuing anyway...${NC}"
fi

echo -e "${GREEN}Calendar Display System Installation${NC}"
echo "=========================================="
echo ""

# Update package lists
echo -e "${GREEN}Updating package lists...${NC}"
apt-get update -qq

# Install required dependencies
echo -e "${GREEN}Installing dependencies...${NC}"
apt-get install -y \
    gcc \
    make \
    git \
    libcurl4-openssl-dev \
    libmicrohttpd-dev \
    libgpiod-dev \
    libssl-dev \
    sqlite3 \
    libsqlite3-dev \
    syslog-ng \
    wget \
    cmake

echo -e "${GREEN}Dependencies installed successfully!${NC}"
echo ""

# Install WiringPi (optional, but useful for GPIO access)
echo -e "${GREEN}Installing WiringPi...${NC}"
if [ ! -f /usr/local/bin/gpio ]; then
    # Check if already installed
    if ! command -v gpio &> /dev/null; then
        # Try to install from package manager first
        if apt-get install -y wiringpi 2>/dev/null; then
            echo "WiringPi installed from package manager"
        else
            echo "WiringPi not available in package manager, skipping"
        fi
    fi
fi

# Create directories
echo -e "${GREEN}Creating directories...${NC}"
mkdir -p /var/lib/calendar
mkdir -p /var/log/calendar
chown pi:pi /var/lib/calendar
chown pi:pi /var/log/calendar

# Copy configuration files
echo -e "${GREEN}Setting up configuration...${NC}"
if [ ! -f /home/pi/.calendar_config.json ]; then
    # Create default config
    cat > /home/pi/.calendar_config.json << 'EOF'
{
  "endpoints": [],
  "refresh_interval": 15,
  "timezone": "UTC",
  "auto_refresh": true
}
EOF
    chown pi:pi /home/pi/.calendar_config.json
fi

# Build the application
echo -e "${GREEN}Building application...${NC}"
cd /workspace/Einwegbecher__C-Cal/calendar-display
make clean
make

if [ ! -f calendar_display ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful!${NC}"
echo ""

# Create systemd service
echo -e "${GREEN}Creating systemd service...${NC}"
cat > /etc/systemd/system/calendar-display.service << 'EOF'
[Unit]
Description=Calendar Display System
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/workspace/Einwegbecher__C-Cal/calendar-display
ExecStart=/workspace/Einwegbecher__C-Cal/calendar-display/calendar_display
Restart=on-failure
RestartSec=5
StandardOutput=syslog
StandardError=syslog
SyslogIdentifier=calendar-display

[Install]
WantedBy=multi-user.target
EOF

# Enable and start the service
echo -e "${GREEN}Setting up service...${NC}"
systemctl daemon-reload
systemctl enable calendar-display.service

# Don't start automatically - let user start it manually
echo ""
echo -e "${GREEN}Installation complete!${NC}"
echo ""
echo "To start the application:"
echo "  sudo systemctl start calendar-display"
echo ""
echo "To view logs:"
echo "  journalctl -u calendar-display -f"
echo ""
echo "To access the web interface:"
echo "  Open http://<raspberry-pi-ip>:8080 in your browser"
echo ""
echo "Wiring instructions for Waveshare 2.15\" G E-Paper:"
echo "  VCC  -> 3.3V (Pin 1)"
echo "  GND  -> GND (Pin 6)"
echo "  DIN  -> MOSI (Pin 19, GPIO 10)"
echo "  CLK  -> SCLK (Pin 23, GPIO 11)"
echo "  CS   -> CE0 (Pin 24, GPIO 8)"
echo "  DC   -> GPIO 25 (Pin 22)"
echo "  RST  -> GPIO 17 (Pin 11)"
echo "  BUSY -> GPIO 24 (Pin 18)"
echo ""
