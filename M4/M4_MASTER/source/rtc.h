#ifndef _RTC_H
#define _RTC_H

#ifdef ext
    #undef ext
    #undef extrd
#endif
#ifdef _RTC_C
  #define ext
  #define extrd
#else
  #define ext extern
  #define extrd extern const
#endif

typedef enum RtcFieldEnum {
    RTC_HUNDREDS,
    RTC_SECONDS,
    RTC_MINUTES,
    RTC_HOURS,
    RTC_DAY,
    RTC_MONTH,
    RTC_YEAR,
    RTC_WEEKDAY
} RtcField;

ext int   rtcInit();
ext int   rtcSetDate(char* date);
ext char* rtcGetDate();
ext char* getWeekdayStr(unsigned char val);
ext int   rtcGet(RtcField field);
#endif // _RTC_H
