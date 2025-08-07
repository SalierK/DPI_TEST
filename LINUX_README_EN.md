# GoodbyeDPI for Linux - User Guide

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](../LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)

## Overview

This is a Linux port of GoodbyeDPI, optimized for Manjaro Linux and other Arch-based distributions. The application bypasses Deep Packet Inspection (DPI) systems to access blocked websites without using a VPN.

## System Requirements

- **OS**: Manjaro Linux, Arch Linux, or other Linux distributions
- **Privileges**: Root privileges (sudo)
- **Dependencies**: 
  - `iptables`
  - `libnetfilter_queue`
  - `gcc` (for building)
  - `make`
  - `pkg-config`

## Quick Installation

### Automatic Installation (Recommended)

```bash
# Download or clone the repository
git clone https://github.com/SalierK/DPI_TEST.git
cd DPI_TEST/src

# Run the installation script
sudo ./install-linux.sh
```

### Manual Installation

1. **Install dependencies** (Manjaro/Arch):
```bash
sudo pacman -S iptables libnetfilter_queue base-devel
```

For Ubuntu/Debian:
```bash
sudo apt update
sudo apt install iptables libnetfilter-queue-dev build-essential pkg-config
```

For CentOS/RHEL/Fedora:
```bash
sudo dnf install iptables libnetfilter_queue-devel gcc make pkg-config
# or for older versions: sudo yum install ...
```

2. **Build the application**:
```bash
cd src
make -f Makefile.linux
```

3. **Install the binary**:
```bash
sudo cp goodbyedpi-linux /usr/local/bin/
sudo chmod +x /usr/local/bin/goodbyedpi-linux
```

## Usage

### Method 1: Systemd Service (Recommended)

After installation via `install-linux.sh`:

```bash
# Start the service
sudo systemctl start goodbyedpi

# Enable auto-start on boot
sudo systemctl enable goodbyedpi

# Check status
sudo systemctl status goodbyedpi

# View logs
sudo journalctl -u goodbyedpi -f
```

### Method 2: Manual Execution

```bash
# Set up iptables rules
sudo iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Run GoodbyeDPI
sudo goodbyedpi-linux --auto --verbose

# To stop, clean up iptables rules
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
```

## Command Line Options

```
Options:
  -p, --port-filter PORT       Filter packets by destination port
  -r, --redirect-dns IP         Redirect DNS queries to specified IP
  -s, --set-ttl TTL             Set custom TTL for packets
  -f, --fragment-http           Fragment HTTP packets
  -e, --fragment-https          Fragment HTTPS packets
  -a, --auto                    Automatically detect best method
  -w, --whitelist FILE          Load domain whitelist from file
  -b, --blacklist FILE          Load domain blacklist from file
  -q, --queue-num NUM           NFQUEUE queue number (default: 0)
  -d, --daemon                  Run as daemon
  -v, --verbose                 Enable verbose output
  -h, --help                    Show help message
      --version                 Show version information
```

## Usage Examples

### Basic Scenarios

```bash
# Auto mode (recommended for most cases)
sudo goodbyedpi-linux --auto --verbose

# Manual configuration with fragmentation and TTL
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64

# With DNS redirection
sudo goodbyedpi-linux --redirect-dns 8.8.8.8 --auto

# Run as daemon
sudo goodbyedpi-linux --auto --daemon

# Filter only specific ports
sudo goodbyedpi-linux --port-filter 443 --fragment-https
```

### Configuration for Specific ISPs

**For most ISPs**:
```bash
sudo goodbyedpi-linux --auto --verbose
```

**For ISPs with aggressive filtering**:
```bash
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64 --redirect-dns 1.1.1.1
```

## iptables Configuration

GoodbyeDPI works with netfilter/iptables to intercept packets. The following rules are automatically set up:

```bash
# Main rule for HTTP/HTTPS traffic
iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Additional rules for other ports (optional)
iptables -t mangle -A OUTPUT -p tcp --dport 8080 -j NFQUEUE --queue-num 0
```

### Manual iptables Configuration

```bash
# Add rule
sudo iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Remove rule
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# View all rules
sudo iptables -t mangle -L OUTPUT -v --line-numbers
```

## Troubleshooting

### Issue: "failed to create netfilter queue handle"

**Solution**: Ensure iptables rules are properly configured and the program is run with root privileges.

```bash
# Check iptables rules
sudo iptables -t mangle -L OUTPUT

# Check that kernel module is loaded
sudo modprobe nfnetlink_queue
```

### Issue: No access to blocked sites

**Solutions**:
1. Try different parameters:
```bash
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64
```

2. Use different DNS:
```bash
sudo goodbyedpi-linux --auto --redirect-dns 8.8.8.8
```

3. Try different queue number:
```bash
sudo goodbyedpi-linux --auto --queue-num 1
```

### Issue: High CPU usage

**Solutions**:
- Use port filtering: `--port-filter 443`
- Limit traffic with more specific iptables rules

### Issue: Application doesn't start

**Checks**:
```bash
# Check dependencies
pkg-config --exists libnetfilter_queue && echo "OK" || echo "Missing libnetfilter_queue"

# Check permissions
ls -l /usr/local/bin/goodbyedpi-linux

# Check system logs
sudo journalctl -u goodbyedpi -n 50
```

## Uninstallation

### Automatic Uninstallation

```bash
cd DPI_TEST/src
sudo ./uninstall-linux.sh
```

### Manual Uninstallation

```bash
# Stop service
sudo systemctl stop goodbyedpi
sudo systemctl disable goodbyedpi

# Remove files
sudo rm /usr/local/bin/goodbyedpi-linux
sudo rm /etc/systemd/system/goodbyedpi.service
sudo systemctl daemon-reload

# Clean up iptables rules
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
```

## Security

- GoodbyeDPI requires root privileges to work with netfilter
- It's recommended to run via systemd service rather than directly
- Regularly update the system and check logs for suspicious activity

## Performance

### Optimization for Low-End Systems

```bash
# Minimal load
sudo goodbyedpi-linux --port-filter 443 --fragment-https

# Without verbose output
sudo goodbyedpi-linux --auto
```

### Performance Monitoring

```bash
# View resource usage
htop -p $(pgrep goodbyedpi-linux)

# Network statistics
ss -tuln | grep :80
ss -tuln | grep :443
```

## Frequently Asked Questions

**Q: Does this work with VPN?**
A: Yes, but usually VPN is not needed when using GoodbyeDPI.

**Q: Does it affect internet speed?**
A: Minimal impact when properly configured. There may be slight latency due to packet processing.

**Q: Are other Linux distributions supported?**
A: Yes, but package installation commands may need to be adapted.

**Q: Can it be used without root privileges?**
A: No, root privileges are required to work with netfilter/iptables.

## Support

- Create an issue in the GitHub repository
- Include in the report: OS version, application logs, parameters used
- For diagnostics use: `sudo goodbyedpi-linux --verbose`

## License

This project is distributed under the Apache 2.0 license. See [LICENSE](../LICENSE) file.

## Development

To contribute to development:

```bash
git clone https://github.com/SalierK/DPI_TEST.git
cd DPI_TEST/src
make -f Makefile.linux debug
```

Main files:
- `goodbyedpi-linux.c` - main application logic
- `linux_packet.c` - packet processing via netfilter
- `Makefile.linux` - Linux build configuration
- `install-linux.sh` - installation script