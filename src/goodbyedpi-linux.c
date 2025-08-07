/*
 * GoodbyeDPI — Passive DPI blocker and Active DPI circumvention utility.
 * Linux version optimized for Manjaro Linux and other distributions.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "linux_compat.h"
#include "linux_packet.h"
#include "goodbyedpi.h"
#include "utils/repl_str.h"

#define GOODBYEDPI_VERSION "v0.2.3rc3-linux"

// Global variables for signal handling
static volatile sig_atomic_t running = 1;

static void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        printf("\nReceived signal %d, shutting down gracefully...\n", signum);
        running = 0;
    }
}

static void print_usage(const char *program_name) {
    printf("GoodbyeDPI %s - Passive DPI blocker and Active DPI circumvention utility (Linux)\n\n", GOODBYEDPI_VERSION);
    printf("Usage: %s [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -p, --port-filter PORT       Filter packets by destination port\n");
    printf("  -r, --redirect-dns IP         Redirect DNS queries to specified IP\n");
    printf("  -s, --set-ttl TTL             Set custom TTL for packets\n");
    printf("  -f, --fragment-http           Fragment HTTP packets\n");
    printf("  -e, --fragment-https          Fragment HTTPS packets\n");
    printf("  -a, --auto                    Automatically detect best method\n");
    printf("  -w, --whitelist FILE          Load domain whitelist from file\n");
    printf("  -b, --blacklist FILE          Load domain blacklist from file\n");
    printf("  -q, --queue-num NUM           NFQUEUE queue number (default: 0)\n");
    printf("  -d, --daemon                  Run as daemon\n");
    printf("  -v, --verbose                 Enable verbose output\n");
    printf("  -h, --help                    Show this help message\n");
    printf("      --version                 Show version information\n\n");
    printf("Examples:\n");
    printf("  %s --auto --verbose\n", program_name);
    printf("  %s --fragment-http --fragment-https --set-ttl 64\n", program_name);
    printf("  %s --redirect-dns 8.8.8.8 --queue-num 1\n", program_name);
    printf("\nNote: This program requires root privileges and proper iptables rules.\n");
    printf("Set up iptables to redirect packets to NFQUEUE, for example:\n");
    printf("  iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0\n");
}

static void print_version(void) {
    printf("GoodbyeDPI %s (Linux)\n", GOODBYEDPI_VERSION);
    printf("Copyright (C) 2024 - DPI circumvention utility for Linux\n");
    printf("This is free software; see the source for copying conditions.\n");
}

int main(int argc, char *argv[]) {
    int opt;
    bool auto_mode = false;
    bool fragment_http = false;
    bool fragment_https = false;
    bool daemon_mode = false;
    bool verbose = false;
    int custom_ttl = 0;
    int queue_num = 0;
    char *dns_redirect = NULL;
    char *whitelist_file = NULL;
    char *blacklist_file = NULL;
    int port_filter = 0;

    static struct option long_options[] = {
        {"auto", no_argument, 0, 'a'},
        {"fragment-http", no_argument, 0, 'f'},
        {"fragment-https", no_argument, 0, 'e'},
        {"daemon", no_argument, 0, 'd'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 0},
        {"port-filter", required_argument, 0, 'p'},
        {"redirect-dns", required_argument, 0, 'r'},
        {"set-ttl", required_argument, 0, 's'},
        {"whitelist", required_argument, 0, 'w'},
        {"blacklist", required_argument, 0, 'b'},
        {"queue-num", required_argument, 0, 'q'},
        {0, 0, 0, 0}
    };

    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "afedvhp:r:s:w:b:q:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                auto_mode = true;
                break;
            case 'f':
                fragment_http = true;
                break;
            case 'e':
                fragment_https = true;
                break;
            case 'd':
                daemon_mode = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'p':
                port_filter = atoi(optarg);
                if (port_filter <= 0 || port_filter > 65535) {
                    fprintf(stderr, "Error: Invalid port number %s\n", optarg);
                    return 1;
                }
                break;
            case 'r':
                dns_redirect = optarg;
                break;
            case 's':
                custom_ttl = atoi(optarg);
                if (custom_ttl <= 0 || custom_ttl > 255) {
                    fprintf(stderr, "Error: Invalid TTL value %s\n", optarg);
                    return 1;
                }
                break;
            case 'w':
                whitelist_file = optarg;
                break;
            case 'b':
                blacklist_file = optarg;
                break;
            case 'q':
                queue_num = atoi(optarg);
                if (queue_num < 0 || queue_num > 65535) {
                    fprintf(stderr, "Error: Invalid queue number %s\n", optarg);
                    return 1;
                }
                break;
            case 0:
                if (strcmp(long_options[optind-1].name, "version") == 0) {
                    print_version();
                    return 0;
                }
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Check if running as root
    if (getuid() != 0) {
        fprintf(stderr, "Error: This program must be run as root.\n");
        fprintf(stderr, "Try: sudo %s [options]\n", argv[0]);
        return 1;
    }

    printf("GoodbyeDPI %s starting...\n", GOODBYEDPI_VERSION);

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Load whitelist/blacklist if specified
    if (whitelist_file) {
        if (verbose) printf("Loading whitelist from %s\n", whitelist_file);
        // TODO: Implement whitelist loading
    }

    if (blacklist_file) {
        if (verbose) printf("Loading blacklist from %s\n", blacklist_file);
        // TODO: Implement blacklist loading
    }

    // Initialize DNS redirection if specified
    if (dns_redirect) {
        if (verbose) printf("Setting up DNS redirection to %s\n", dns_redirect);
        // TODO: Implement DNS redirection
    }

    // Auto-mode configuration
    if (auto_mode) {
        if (verbose) printf("Auto-mode enabled: using optimal settings\n");
        fragment_http = true;
        fragment_https = true;
        custom_ttl = 64;
    }

    // Display configuration
    printf("Configuration:\n");
    printf("  Queue number: %d\n", queue_num);
    printf("  Fragment HTTP: %s\n", fragment_http ? "yes" : "no");
    printf("  Fragment HTTPS: %s\n", fragment_https ? "yes" : "no");
    if (custom_ttl > 0) printf("  Custom TTL: %d\n", custom_ttl);
    if (port_filter > 0) printf("  Port filter: %d\n", port_filter);
    if (dns_redirect) printf("  DNS redirect: %s\n", dns_redirect);
    printf("  Verbose mode: %s\n", verbose ? "yes" : "no");
    printf("  Daemon mode: %s\n", daemon_mode ? "yes" : "no");

    // Daemonize if requested
    if (daemon_mode) {
        if (verbose) printf("Entering daemon mode...\n");
        if (daemon(0, 0) != 0) {
            perror("daemon");
            return 1;
        }
    }

    // Initialize packet capture
    if (linux_packet_init(queue_num) != 0) {
        fprintf(stderr, "Error: Failed to initialize packet capture\n");
        return 1;
    }

    printf("GoodbyeDPI is running. Press Ctrl+C to stop.\n");

    // Main packet processing loop
    while (running) {
        if (linux_packet_loop(1000) != 0) {
            if (running) {
                fprintf(stderr, "Error in packet processing loop\n");
                break;
            }
        }
    }

    // Cleanup
    printf("Shutting down...\n");
    linux_packet_cleanup();
    printf("GoodbyeDPI stopped.\n");

    return 0;
}