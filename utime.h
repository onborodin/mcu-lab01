
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

typedef struct tm tm_t;

#if 0
typedef struct tm {
    uint8_t tm_sec;
    uint8_t tm_min;
    uint8_t tm_hour;
    uint8_t tm_mday;
    uint8_t tm_mon;
    uint16_t tm_year;
} tm_t;

#endif
