#ifndef __MBED_RTC_H__
#define __MBED_RTC_H__

void platform_rtc_setalarmdatetime( int day, int month, int year, int hour, int min, int sec );
void platform_rtc_setdatetime( int day, int month, int year, int hour, int min, int sec );
void platform_rtc_getdatetime( int* day, int* month, int* year, int* hour, int* min, int* sec  );

#endif
