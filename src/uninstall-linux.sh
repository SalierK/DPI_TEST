#!/bin/bash
# GoodbyeDPI Linux Uninstallation Script
# This script removes GoodbyeDPI and cleans up iptables rules

set -e

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

stop_service() {
    print_info "Stopping GoodbyeDPI service..."
    
    if systemctl is-active --quiet goodbyedpi; then
        systemctl stop goodbyedpi
        print_success "Service stopped"
    else
        print_warning "Service was not running"
    fi
    
    if systemctl is-enabled --quiet goodbyedpi; then
        systemctl disable goodbyedpi
        print_success "Service disabled"
    fi
}

remove_service() {
    print_info "Removing systemd service..."
    
    if [[ -f "$SERVICE_DIR/goodbyedpi.service" ]]; then
        rm -f "$SERVICE_DIR/goodbyedpi.service"
        systemctl daemon-reload
        print_success "Service file removed"
    else
        print_warning "Service file not found"
    fi
}

remove_binary() {
    print_info "Removing GoodbyeDPI binary..."
    
    if [[ -f "$INSTALL_DIR/goodbyedpi-linux" ]]; then
        rm -f "$INSTALL_DIR/goodbyedpi-linux"
        print_success "Binary removed"
    else
        print_warning "Binary not found"
    fi
}

cleanup_iptables() {
    print_info "Cleaning up iptables rules..."
    
    # Remove iptables rule
    if iptables -t mangle -C OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 &>/dev/null; then
        iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
        print_success "iptables rule removed"
    else
        print_warning "iptables rule not found"
    fi
}

main() {
    echo "GoodbyeDPI Linux Uninstallation Script"
    echo "======================================"
    echo
    
    check_root
    stop_service
    remove_service
    remove_binary
    cleanup_iptables
    
    print_success "GoodbyeDPI has been completely removed from your system"
    echo
    print_info "Note: Any custom iptables rules you added manually were not removed"
}

main "$@"