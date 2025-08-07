/*
 * Linux compatibility layer for Windows types and functions
 * This header provides Linux equivalents for Windows-specific types
 */

#ifndef LINUX_COMPAT_H
#define LINUX_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

// Windows type definitions mapped to Linux equivalents
typedef uint32_t UINT;
typedef uint8_t BYTE;
typedef bool BOOL;
typedef void* HANDLE;
typedef void* PWINDIVERT_ADDRESS;

// Boolean constants
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Linux daemon function declaration
#define _GNU_SOURCE
#include <unistd.h>

#endif /* LINUX_COMPAT_H */