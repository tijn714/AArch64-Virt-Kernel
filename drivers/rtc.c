#include "common/common.h"

#define PL031_RTC_BASE 0x09010000
#define RTC_DR (*(volatile uint32_t *)(PL031_RTC_BASE + 0x00)) // Data Register

uint32_t rtc_get_epoch(void) {
    return RTC_DR;
}
