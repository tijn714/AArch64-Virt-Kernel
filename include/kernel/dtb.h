#ifndef DTB_H
#define DTB_H

#include <common/common.h>
#include <lib/string.h>
#include <kernel/uart.h>

#define MAX_DEVICES 128
#define MAX_DEVICE_NAME 64
#define MAX_PROP_VALUE 128
#define MAX_REG_SIZE 64       // Max bytes for raw reg property
#define MAX_OTHER_PROPS 16     // Max number of additional properties
#define MAX_PROP_NAME 32      // Max length of property name

// Structure for additional properties
typedef struct {
    char name[MAX_PROP_NAME];
    uint8_t value[MAX_PROP_VALUE];
    uint32_t len;
} prop_t;

// Device structure to hold module/device information
typedef struct {
    bool is_memory;
    char name[MAX_DEVICE_NAME];
    char compatible[MAX_PROP_VALUE];
    uint8_t regs[MAX_REG_SIZE]; // Raw reg property data
    uint32_t regs_len;          // Length of reg data
    prop_t other_props[MAX_OTHER_PROPS]; // Other properties
    uint32_t prop_count;        // Number of other properties
    bool is_active;
} device_t;

// DTB parser context
typedef struct {
    device_t devices[MAX_DEVICES];
    uint32_t device_count;
} dtb_ctx;

// Initialize and parse the DTB
void dtb_parse(const void *dtb_blob, dtb_ctx *ctx);

// Print all devices
void dtb_print_devices(const dtb_ctx *ctx);

#endif // DTB_H