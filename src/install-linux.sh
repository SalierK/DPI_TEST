#!/bin/bash
# GoodbyeDPI Linux Setup Script for Manjaro Linux
# This script sets up iptables rules and installs GoodbyeDPI

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/usr/local/bin"
SERVICE_DIR="/etc/systemd/system"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_root() {
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run as root"
        echo "Usage: sudo $0"
        exit 1
    fi
}

check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for iptables
    if ! command -v iptables &> /dev/null; then
        print_error "iptables is not installed"
        print_info "Install with: sudo pacman -S iptables"
        exit 1
    fi
    
    # Check for libnetfilter_queue
    if ! pkg-config --exists libnetfilter_queue; then
        print_error "libnetfilter_queue is not installed"
        print_info "Install with: sudo pacman -S libnetfilter_queue"
        exit 1
    fi
    
    print_success "All dependencies are satisfied"
}

build_goodbyedpi() {
    print_info "Building GoodbyeDPI for Linux..."
    cd "$SCRIPT_DIR"
    
    if [[ ! -f "Makefile.linux" ]]; then
        print_error "Makefile.linux not found in $SCRIPT_DIR"
        exit 1
    fi
    
    make -f Makefile.linux clean
    make -f Makefile.linux
    
    if [[ ! -f "goodbyedpi-linux" ]]; then
        print_error "Build failed - goodbyedpi-linux executable not found"
        exit 1
    fi
    
    print_success "Build completed successfully"
}

install_binary() {
    print_info "Installing GoodbyeDPI binary..."
    
    cp "$SCRIPT_DIR/goodbyedpi-linux" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/goodbyedpi-linux"
    
    print_success "Binary installed to $INSTALL_DIR/goodbyedpi-linux"
}

create_service() {
    print_info "Creating systemd service..."
    
    cat > "$SERVICE_DIR/goodbyedpi.service" << EOF
[Unit]
Description=GoodbyeDPI - DPI circumvention utility for Linux
Documentation=https://github.com/SalierK/DPI_TEST
After=network.target
Wants=network.target

[Service]
Type=simple
ExecStartPre=/bin/bash -c 'iptables -t mangle -C OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 &>/dev/null || iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0'
ExecStart=$INSTALL_DIR/goodbyedpi-linux --auto --verbose
ExecStopPost=/bin/bash -c 'iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 &>/dev/null || true'
Restart=on-failure
RestartSec=5
User=root

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    print_success "Systemd service created"
}

setup_iptables_rules() {
    print_info "Setting up iptables rules..."
    
    # Add rule to redirect HTTP/HTTPS traffic to NFQUEUE
    if ! iptables -t mangle -C OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 &>/dev/null; then
        iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
        print_success "iptables rule added"
    else
        print_warning "iptables rule already exists"
    fi
}

show_usage() {
    print_success "Installation completed successfully!"
    echo
    print_info "Usage:"
    echo "  Start service: sudo systemctl start goodbyedpi"
    echo "  Enable on boot: sudo systemctl enable goodbyedpi"
    echo "  Check status: sudo systemctl status goodbyedpi"
    echo "  View logs: sudo journalctl -u goodbyedpi -f"
    echo
    echo "  Manual usage: sudo goodbyedpi-linux --help"
    echo
    print_info "Configuration:"
    echo "  Service file: $SERVICE_DIR/goodbyedpi.service"
    echo "  Binary location: $INSTALL_DIR/goodbyedpi-linux"
    echo
    print_warning "Note: The service will automatically manage iptables rules"
}

main() {
    echo "GoodbyeDPI Linux Installation Script for Manjaro"
    echo "================================================="
    echo
    
    check_root
    check_dependencies
    build_goodbyedpi
    install_binary
    create_service
    setup_iptables_rules
    show_usage
}

main "$@"