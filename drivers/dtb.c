#include <common/common.h>
#include <lib/string.h>
#include <kernel/uart.h>
#include <kernel/dtb.h>

// DTB magic number and structure definitions
#define DTB_MAGIC 0xd00dfeed
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

// DTB header structure
typedef struct {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} fdt_header_t;

// Property structure
typedef struct {
    uint32_t len;
    uint32_t nameoff;
} fdt_prop_t;

// Swap big-endian to host byte order
static inline uint32_t be32_to_cpu(uint32_t val) {
    return ((val & 0xff000000) >> 24) |
           ((val & 0x00ff0000) >> 8)  |
           ((val & 0x0000ff00) << 8)  |
           ((val & 0x000000ff) << 24);
}

// Align to 4-byte boundary
static inline uint32_t align(uint32_t val) {
    return (val + 3) & ~3;
}

// Check device type based on compatible string or device_type
static const char *get_device_type(const device_t *dev) {
    if (dev->is_memory) {
        return "Memory";
    }
    if (strcmp(dev->compatible, "arm,pl031") == 0) {
        return "RTC";
    }
    if (strcmp(dev->compatible, "arm,pl011") == 0) {
        return "UART";
    }
    if (strcmp(dev->compatible, "virtio,gpu") == 0) {
        return "GPU";
    }
    for (uint32_t i = 0; i < dev->prop_count; i++) {
        if (strcmp(dev->other_props[i].name, "device_type") == 0 &&
            dev->other_props[i].len > 0 &&
            dev->other_props[i].value[dev->other_props[i].len - 1] == '\0') {
            return (const char *)dev->other_props[i].value;
        }
    }
    return "Unknown";
}

// Parse a single node
static const void *parse_node(const void *dtb, const char *strings, dtb_ctx *ctx, const void *end) {
    const uint32_t *ptr = dtb;
    device_t *dev = NULL;

    if (ctx->device_count < MAX_DEVICES) {
        dev = &ctx->devices[ctx->device_count];
        dev->is_memory = false;
        dev->name[0] = '\0';
        dev->compatible[0] = '\0';
        dev->regs_len = 0;
        dev->prop_count = 0;
        dev->is_active = true;
    }

    // Read node name
    const char *name = (const char *)ptr;
    uint32_t name_len = strlen(name) + 1;
    if (dev && name_len <= MAX_DEVICE_NAME) {
        strncpy(dev->name, name, MAX_DEVICE_NAME - 1);
        dev->name[MAX_DEVICE_NAME - 1] = '\0';
    }
    ptr = (const uint32_t *)((const char *)ptr + align(name_len));

    // Process tokens
    while ((const void *)ptr < end) {
        uint32_t token = be32_to_cpu(*ptr++);
        if (token == FDT_END_NODE) {
            break;
        } else if (token == FDT_NOP) {
            continue;
        } else if (token == FDT_BEGIN_NODE) {
            ptr = (const uint32_t *)parse_node(ptr, strings, ctx, end);
        } else if (token == FDT_PROP) {
            fdt_prop_t *prop = (fdt_prop_t *)ptr;
            uint32_t len = be32_to_cpu(prop->len);
            if (len == 0) {
                ptr += 2;
                ptr = (const uint32_t *)((const char *)ptr + align(len));
                continue;
            }
            uint32_t nameoff = be32_to_cpu(prop->nameoff);
            ptr += 2; // Skip len and nameoff
            const char *prop_name = strings + nameoff;
            const uint8_t *prop_val = (const uint8_t *)ptr;

            if (dev) {
                if (strcmp(prop_name, "compatible") == 0 && len <= MAX_PROP_VALUE) {
                    strncpy(dev->compatible, (const char *)prop_val, MAX_PROP_VALUE - 1);
                    dev->compatible[MAX_PROP_VALUE - 1] = '\0';
                } else if (strcmp(prop_name, "device_type") == 0 && len <= MAX_PROP_VALUE) {
                    if (strncmp((const char *)prop_val, "memory", len) == 0) {
                        dev->is_memory = true;
                    }
                    if (dev->prop_count < MAX_OTHER_PROPS) {
                        prop_t *other = &dev->other_props[dev->prop_count];
                        strncpy(other->name, prop_name, MAX_PROP_NAME - 1);
                        other->name[MAX_PROP_NAME - 1] = '\0';
                        memcpy(other->value, prop_val, len);
                        other->len = len;
                        dev->prop_count++;
                    }
                } else if (strcmp(prop_name, "reg") == 0 && len <= MAX_REG_SIZE) {
                    dev->regs_len = len;
                    memcpy(dev->regs, prop_val, len);
                } else if (strcmp(prop_name, "status") == 0 && len <= MAX_PROP_VALUE) {
                    if (strncmp((const char *)prop_val, "disabled", len) == 0) {
                        dev->is_active = false;
                    }
                    if (dev->prop_count < MAX_OTHER_PROPS) {
                        prop_t *other = &dev->other_props[dev->prop_count];
                        strncpy(other->name, prop_name, MAX_PROP_NAME - 1);
                        other->name[MAX_PROP_NAME - 1] = '\0';
                        memcpy(other->value, prop_val, len);
                        other->len = len;
                        dev->prop_count++;
                    }
                } else if (dev->prop_count < MAX_OTHER_PROPS && len <= MAX_PROP_VALUE) {
                    prop_t *other = &dev->other_props[dev->prop_count];
                    strncpy(other->name, prop_name, MAX_PROP_NAME - 1);
                    other->name[MAX_PROP_NAME - 1] = '\0';
                    memcpy(other->value, prop_val, len);
                    other->len = len;
                    dev->prop_count++;
                }
            }
            ptr = (const uint32_t *)((const char *)ptr + align(len));
        } else if (token == FDT_END) {
            return ptr;
        }
    }

    if (dev && (dev->name[0] != '\0' || dev->compatible[0] != '\0' || dev->is_memory || dev->regs_len > 0 || dev->prop_count > 0)) {
        ctx->device_count++;
    } else if (dev) {
        uart_printk("Skipped node with empty name at %p\n", ptr);
    }

    return ptr;
}

// Parse the DTB blob
void dtb_parse(const void *dtb_blob, dtb_ctx *ctx) {
    const fdt_header_t *header = (const fdt_header_t *)dtb_blob;
    if (be32_to_cpu(header->magic) != DTB_MAGIC) {
        uart_printk("Invalid DTB magic number\n");
        return;
    }

    ctx->device_count = 0;
    const void *dt_struct = (const char *)dtb_blob + be32_to_cpu(header->off_dt_struct);
    const char *dt_strings = (const char *)dtb_blob + be32_to_cpu(header->off_dt_strings);
    const void *end = (const char *)dtb_blob + be32_to_cpu(header->totalsize);

    parse_node(dt_struct, dt_strings, ctx, end);
}

// Print all devices
void dtb_print_devices(const dtb_ctx *ctx) {
    for (uint32_t i = 0; i < ctx->device_count; i++) {
        const device_t *dev = &ctx->devices[i];
        const char *type = get_device_type(dev);
        uart_printk("Device %u: Name=%s, Type=%s, Active=%s\n", i, dev->name, type, dev->is_active ? "yes" : "no");
        if (dev->compatible[0] != '\0') {
            uart_printk("  Compatible: %s\n", dev->compatible);
        }
        if (dev->regs_len > 0) {
            uart_printk("  Regs: [");
            for (uint32_t j = 0; j < dev->regs_len; j++) {
                uart_printk("%02x", dev->regs[j]);
                if (j < dev->regs_len - 1) uart_printk(" ");
            }
            uart_printk("]\n");
        }
        for (uint32_t j = 0; j < dev->prop_count; j++) {
            const prop_t *prop = &dev->other_props[j];
            uart_printk("  %s: ", prop->name);
            if (prop->len > 0 && prop->value[0] >= 32 && prop->value[0] <= 126) {
                uint32_t offset = 0;
                bool first = true;
                while (offset < prop->len) {
                    if (!first) uart_printk(", ");
                    uart_printk("%s", (const char *)(prop->value + offset));
                    offset += strlen((const char *)(prop->value + offset)) + 1;
                    first = false;
                }
                uart_printk("\n");
            } else {
                uart_printk("[");
                for (uint32_t k = 0; k < prop->len; k++) {
                    uart_printk("%02x", prop->value[k]);
                    if (k < prop->len - 1) uart_printk(" ");
                }
                uart_printk("]\n");
            }
        }
    }
}