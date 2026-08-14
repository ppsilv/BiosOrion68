#ifndef __0RTC_H__
#define __0RTC_H__

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t mday;
    uint8_t mon;
    uint8_t year; /* 00-99 */
} __attribute__((packed))  rtc_time_t;

extern void sys_rtc_read_time(rtc_time_t *t);
extern void sys_rtc_set_time(rtc_time_t *t);
extern uint8_t sys_eeprom_read_byte(uint16_t addr);
extern void sys_eeprom_write_byte(uint16_t addr, uint8_t data);


#endif
