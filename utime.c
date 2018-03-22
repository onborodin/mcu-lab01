/* $Id$ */

#include <stdint.h>
#include <time.h>
#include <utime.h>

/*
  DEF: standard signed format
  UNDEF: non-standard unsigned format
 */
//#define XT_SIGNED

#ifdef XT_SIGNED
typedef int32_t xtime_t;
#else
typedef uint32_t xtime_t;
#endif

#define TBIAS_DAYS ((70 * ( uint32_t )365) + 17)
#define TBIAS_SECS (TBIAS_DAYS * (xtime_t)86400)
#define TBIAS_YEAR 1900
#define MONTAB(year) ((((year) & 03) || ((year) == 0)) ? mos : lmos)

const int16_t lmos[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
const int16_t mos[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

#define DAYSTO32(year, mon) (((year - 1) / 4) + MONTAB(year)[mon])

/* Convert time structure to scalar time */
xtime_t xtmtot(tm_t *t) {
    int32_t days;
    xtime_t secs;
    int32_t mon, year;

    /* Calculate number of days. */
    mon = t->tm_mon - 1;
    year = t->tm_year - TBIAS_YEAR;
    days = DAYSTO32(year, mon) - 1;
    days += 365 * year;
    days += t->tm_mday;
    days -= TBIAS_DAYS;

    /* Calculate number of seconds. */
    secs = 3600 * t->tm_hour;
    secs += 60 * t->tm_min;
    secs += t->tm_sec;

    secs += (days * (xtime_t) 86400);

    return (secs);
}

void xttotm(tm_t *t, xtime_t secsarg) {
    uint32_t secs;
    int32_t days;
    int32_t mon;
    int32_t year;
    int32_t i;
    const int16_t *pm;

#ifdef XT_SIGNED
    if (secsarg >= 0) {
        secs = (uint32_t) secsarg;
        days = TBIAS_DAYS;
    } else {
        secs = (uint32_t) secsarg + TBIAS_SECS;
        days = 0;
    }
#else
    secs = secsarg;
    days = TBIAS_DAYS;
#endif

    /* days, hour, min, sec */
    days += secs / 86400;
    secs = secs % 86400;
    t->tm_hour = secs / 3600;
    secs %= 3600;
    t->tm_min = secs / 60;
    t->tm_sec = secs % 60;

    /* determine year */
    for (year = days / 365; days < (i = DAYSTO32(year, 0) + 365 * year);) {
        --year;
    }
    days -= i;
    t->tm_year = year + TBIAS_YEAR;

    /* determine month */
    pm = MONTAB(year);
    for (mon = 12; days < pm[--mon];);
    t->tm_mon = mon + 1;
    t->tm_mday = days - pm[mon] + 1;
}

/* EOF */

