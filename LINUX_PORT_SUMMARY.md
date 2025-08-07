# GoodbyeDPI Linux Port - Summary

## Overview

This repository now contains a complete Linux port of GoodbyeDPI, optimized specifically for Manjaro Linux and other Linux distributions. The port replaces Windows-specific WinDivert functionality with Linux netfilter/iptables mechanisms while maintaining the core DPI bypass functionality.

## What's New

### ✅ Complete Linux Implementation
- **Native Linux binary**: `goodbyedpi-linux` with full POSIX compatibility
- **Netfilter integration**: Uses `libnetfilter_queue` instead of WinDivert
- **Systemd service**: Proper Linux service management
- **Automated installation**: One-command setup for Manjaro/Arch Linux

### ✅ Key Components Added

1. **Core Linux Implementation**
   - `src/goodbyedpi-linux.c` - Main application with Linux-specific code
   - `src/linux_packet.c/h` - Packet capture using netfilter queues
   - `src/linux_compat.h` - Windows type compatibility layer

2. **Build System**
   - `src/Makefile.linux` - Optimized Linux build configuration
   - Dependency checking with pkg-config
   - Security hardening flags

3. **Installation & Management**
   - `src/install-linux.sh` - Automated installation script
   - `src/uninstall-linux.sh` - Clean removal script
   - `src/quick-start.sh` - Easy testing without full installation
   - Systemd service configuration

4. **Configuration System**
   - `src/config.c/h` - Configuration file loader
   - `src/goodbyedpi.conf` - Example configuration with ISP presets
   - Command-line and config file support

5. **Documentation**
   - `LINUX_README.md` - Comprehensive Russian documentation
   - `LINUX_README_EN.md` - Complete English documentation
   - Installation guides, troubleshooting, FAQ

## Usage

### Quick Start (Testing)
```bash
git clone https://github.com/SalierK/DPI_TEST.git
cd DPI_TEST/src
sudo ./quick-start.sh --auto
```

### Full Installation (Manjaro/Arch)
```bash
cd DPI_TEST/src
sudo ./install-linux.sh
sudo systemctl start goodbyedpi
sudo systemctl enable goodbyedpi
```

### Manual Usage
```bash
# Set iptables rule
sudo iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Run GoodbyeDPI
sudo ./goodbyedpi-linux --auto --verbose

# Clean up
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
```

## Key Features

### 🚀 Performance Optimizations
- **CPU optimization**: Minimal processing overhead
- **Memory efficient**: Low memory footprint
- **Network optimized**: Configurable buffer sizes
- **Port filtering**: Process only specific ports to reduce load

### 🔧 Configuration Options
- **Auto-detection mode**: Automatically select best bypass method
- **Manual configuration**: Fine-tune packet fragmentation, TTL, DNS
- **ISP presets**: Pre-configured settings for common providers
- **Config files**: Store settings in `/etc/goodbyedpi.conf`

### 🛡️ Security Features
- **Root privilege requirement**: Secure operation
- **Systemd integration**: Proper service isolation
- **Clean shutdown**: Automatic iptables cleanup
- **Logging**: Comprehensive logging for debugging

### 📦 Easy Installation
- **One-command install**: Automated dependency management
- **Service management**: Start/stop/enable with systemctl
- **Clean uninstall**: Complete removal with cleanup

## Technical Details

### Dependencies
- `libnetfilter_queue` - Packet capture and injection
- `iptables` - Traffic redirection to NFQUEUE
- `gcc`, `make`, `pkg-config` - Build tools

### Compatibility
- **Primary**: Manjaro Linux, Arch Linux
- **Tested**: Ubuntu, Debian (with apt package manager)
- **Supported**: Any Linux with netfilter support

### Architecture
```
User Space:     goodbyedpi-linux (NFQUEUE consumer)
                         ↕
Kernel Space:   netfilter/iptables (packet interception)
                         ↕
Network:        Outgoing TCP packets (HTTP/HTTPS)
```

## Compared to Windows Version

| Feature | Windows (Original) | Linux (This Port) |
|---------|-------------------|-------------------|
| Packet Capture | WinDivert | libnetfilter_queue |
| Service Management | Windows Service | systemd |
| Installation | ZIP + batch files | install script + package manager |
| Configuration | Command line only | Config files + command line |
| Dependency Management | Manual | Automated with pkg-config |
| Root Requirements | Admin rights | sudo/root |

## Future Improvements

The current implementation provides core DPI bypass functionality. Future enhancements could include:

- [ ] Port remaining Windows modules (DNS redirection, blacklists)
- [ ] GUI configuration tool
- [ ] More ISP-specific presets
- [ ] IPv6 support enhancement
- [ ] Performance monitoring tools
- [ ] Docker container support

## Support

For issues, questions, or contributions:
1. Check the comprehensive documentation in `LINUX_README.md`
2. Review troubleshooting section for common issues
3. Create GitHub issue with system details and logs
4. Use `sudo goodbyedpi-linux --verbose` for debugging

## License

This Linux port maintains the original Apache 2.0 license and is fully open source.