/*
 * Configuration header for GoodbyeDPI Linux
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    bool auto_mode;
    bool verbose;
    bool daemon;
    int queue_num;
    bool fragment_http;
    bool fragment_https;
    int custom_ttl;
    int port_filter;
    char dns_redirect[64];
    char whitelist_file[256];
    char blacklist_file[256];
} goodbyedpi_config_t;

// Load configuration from file
int load_config(const char* config_file, goodbyedpi_config_t* config);

// Print current configuration
void print_config(const goodbyedpi_config_t* config);

#endif /* CONFIG_H */