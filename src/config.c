/*
 * Configuration loader for GoodbyeDPI Linux
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <strings.h>
#include "config.h"

// Default configuration
goodbyedpi_config_t default_config = {
    .auto_mode = true,
    .verbose = false,
    .daemon = false,
    .queue_num = 0,
    .fragment_http = false,
    .fragment_https = false,
    .custom_ttl = 0,
    .port_filter = 0,
    .dns_redirect = {0},
    .whitelist_file = {0},
    .blacklist_file = {0}
};

// Trim whitespace from string
static char* trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

// Parse boolean value
static bool parse_bool(const char* value) {
    if (strcasecmp(value, "true") == 0 || 
        strcasecmp(value, "yes") == 0 || 
        strcasecmp(value, "1") == 0) {
        return true;
    }
    return false;
}

// Load configuration from file
int load_config(const char* config_file, goodbyedpi_config_t* config) {
    FILE* file;
    char line[512];
    char key[128], value[384];
    
    // Start with default configuration
    *config = default_config;
    
    file = fopen(config_file, "r");
    if (!file) {
        // Config file not found, use defaults
        return 0;
    }
    
    printf("Loading configuration from %s\n", config_file);
    
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        
        // Skip empty lines and comments
        if (*trimmed == '\0' || *trimmed == '#') {
            continue;
        }
        
        // Parse key=value pairs
        if (sscanf(trimmed, "%127[^=]=%383[^\n]", key, value) == 2) {
            char* trimmed_key = trim_whitespace(key);
            char* trimmed_value = trim_whitespace(value);
            
            // Remove quotes if present
            if (trimmed_value[0] == '"' && trimmed_value[strlen(trimmed_value)-1] == '"') {
                trimmed_value[strlen(trimmed_value)-1] = '\0';
                trimmed_value++;
            }
            
            // Parse configuration options
            if (strcmp(trimmed_key, "AUTO_MODE") == 0) {
                config->auto_mode = parse_bool(trimmed_value);
            } else if (strcmp(trimmed_key, "VERBOSE") == 0) {
                config->verbose = parse_bool(trimmed_value);
            } else if (strcmp(trimmed_key, "DAEMON") == 0) {
                config->daemon = parse_bool(trimmed_value);
            } else if (strcmp(trimmed_key, "QUEUE_NUM") == 0) {
                config->queue_num = atoi(trimmed_value);
            } else if (strcmp(trimmed_key, "FRAGMENT_HTTP") == 0) {
                config->fragment_http = parse_bool(trimmed_value);
            } else if (strcmp(trimmed_key, "FRAGMENT_HTTPS") == 0) {
                config->fragment_https = parse_bool(trimmed_value);
            } else if (strcmp(trimmed_key, "CUSTOM_TTL") == 0) {
                config->custom_ttl = atoi(trimmed_value);
            } else if (strcmp(trimmed_key, "PORT_FILTER") == 0) {
                config->port_filter = atoi(trimmed_value);
            } else if (strcmp(trimmed_key, "DNS_REDIRECT") == 0) {
                if (strlen(trimmed_value) > 0) {
                    strncpy(config->dns_redirect, trimmed_value, sizeof(config->dns_redirect) - 1);
                }
            } else if (strcmp(trimmed_key, "WHITELIST_FILE") == 0) {
                if (strlen(trimmed_value) > 0) {
                    strncpy(config->whitelist_file, trimmed_value, sizeof(config->whitelist_file) - 1);
                }
            } else if (strcmp(trimmed_key, "BLACKLIST_FILE") == 0) {
                if (strlen(trimmed_value) > 0) {
                    strncpy(config->blacklist_file, trimmed_value, sizeof(config->blacklist_file) - 1);
                }
            }
        }
    }
    
    fclose(file);
    return 1;
}

void print_config(const goodbyedpi_config_t* config) {
    printf("Configuration:\n");
    printf("  Auto mode: %s\n", config->auto_mode ? "yes" : "no");
    printf("  Verbose: %s\n", config->verbose ? "yes" : "no");
    printf("  Daemon: %s\n", config->daemon ? "yes" : "no");
    printf("  Queue number: %d\n", config->queue_num);
    printf("  Fragment HTTP: %s\n", config->fragment_http ? "yes" : "no");
    printf("  Fragment HTTPS: %s\n", config->fragment_https ? "yes" : "no");
    if (config->custom_ttl > 0) {
        printf("  Custom TTL: %d\n", config->custom_ttl);
    }
    if (config->port_filter > 0) {
        printf("  Port filter: %d\n", config->port_filter);
    }
    if (strlen(config->dns_redirect) > 0) {
        printf("  DNS redirect: %s\n", config->dns_redirect);
    }
    if (strlen(config->whitelist_file) > 0) {
        printf("  Whitelist file: %s\n", config->whitelist_file);
    }
    if (strlen(config->blacklist_file) > 0) {
        printf("  Blacklist file: %s\n", config->blacklist_file);
    }
}