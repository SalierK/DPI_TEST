#!/bin/bash
# Quick Start Script for GoodbyeDPI Linux
# This script provides an easy way to test GoodbyeDPI without full installation

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

check_root() {
    if [[ $EUID -ne 0 ]]; then
        print_error "This script must be run as root"
        echo "Usage: sudo $0"
        exit 1
    fi
}

check_and_build() {
    cd "$SCRIPT_DIR"
    
    if [[ ! -f "goodbyedpi-linux" ]]; then
        print_info "Building GoodbyeDPI for Linux..."
        if ! make -f Makefile.linux >/dev/null 2>&1; then
            print_error "Build failed. Please install dependencies:"
            echo "  Manjaro/Arch: sudo pacman -S libnetfilter_queue base-devel"
            echo "  Ubuntu/Debian: sudo apt install libnetfilter-queue-dev build-essential"
            exit 1
        fi
        print_success "Build completed"
    fi
}

setup_temp_rules() {
    print_info "Setting up temporary iptables rules..."
    
    # Check if rule already exists
    if ! iptables -t mangle -C OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 2>/dev/null; then
        iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
        echo "1" > /tmp/goodbyedpi_rule_added
        print_success "iptables rule added"
    else
        print_warning "iptables rule already exists"
        echo "0" > /tmp/goodbyedpi_rule_added
    fi
}

cleanup_rules() {
    if [[ -f /tmp/goodbyedpi_rule_added ]] && [[ "$(cat /tmp/goodbyedpi_rule_added)" == "1" ]]; then
        print_info "Cleaning up iptables rules..."
        iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0 2>/dev/null
        rm -f /tmp/goodbyedpi_rule_added
        print_success "iptables rules cleaned up"
    fi
}

# Trap to ensure cleanup on exit
trap cleanup_rules EXIT INT TERM

show_usage() {
    echo "GoodbyeDPI Linux Quick Start"
    echo "============================"
    echo
    echo "Usage: sudo $0 [options]"
    echo
    echo "Options:"
    echo "  --auto              Run with auto-detection (default)"
    echo "  --fragment          Run with packet fragmentation"
    echo "  --dns IP            Use custom DNS server"
    echo "  --help              Show this help"
    echo
    echo "Examples:"
    echo "  sudo $0                           # Auto mode"
    echo "  sudo $0 --fragment                # With fragmentation"
    echo "  sudo $0 --dns 8.8.8.8            # Custom DNS"
    echo "  sudo $0 --fragment --dns 1.1.1.1 # Combined options"
    echo
}

main() {
    local MODE="--auto"
    local DNS=""
    local EXTRA_ARGS=""
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help)
                show_usage
                exit 0
                ;;
            --auto)
                MODE="--auto"
                shift
                ;;
            --fragment)
                MODE="--fragment-http --fragment-https"
                shift
                ;;
            --dns)
                DNS="--redirect-dns $2"
                shift 2
                ;;
            *)
                EXTRA_ARGS="$EXTRA_ARGS $1"
                shift
                ;;
        esac
    done
    
    echo "GoodbyeDPI Linux Quick Start"
    echo "============================"
    echo
    
    check_root
    check_and_build
    setup_temp_rules
    
    print_info "Starting GoodbyeDPI with options: $MODE $DNS $EXTRA_ARGS"
    print_warning "Press Ctrl+C to stop"
    echo
    
    ./goodbyedpi-linux $MODE $DNS --verbose $EXTRA_ARGS
}

main "$@"