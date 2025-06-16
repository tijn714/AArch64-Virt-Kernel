#ifndef RTC_H
#define RTC_H

#include <common/common.h>

void rtc_init(void *base);
uint32_t rtc_get_epoch(void);

#endif
