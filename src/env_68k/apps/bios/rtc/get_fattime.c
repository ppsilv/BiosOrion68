#include "ff.h"

/* Estrutura de exemplo para os dados do seu RTC */
typedef struct {
    uint16_t year;  /* ex: 2026 */
    uint8_t  month; /* 1..12 */
    uint8_t  day;   /* 1..31 */
    uint8_t  hour;  /* 0..23 */
    uint8_t  min;   /* 0..59 */
    uint8_t  sec;   /* 0..59 */
} RTC_Time;

/* Substitua pela chamada real do seu driver de RTC */
extern void rtc_read(RTC_Time *t);

/* Função obrigatoriamente chamada pelo FatFs */
DWORD get_fattime (void) {
    RTC_Time t;
    
    rtc_read(&t);

    return ((DWORD)(t.year - 1980) << 25)
         | ((DWORD)t.month << 21)
         | ((DWORD)t.day << 16)
         | ((DWORD)t.hour << 11)
         | ((DWORD)t.min << 5)
         | ((DWORD)(t.sec / 2));
}