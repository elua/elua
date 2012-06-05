#ifndef __MBED_RTC_H__
#define __MBED_RTC_H__

static void platform_rtc_gettime( int* hour, int* min, int* sec );
static void platform_rtc_settime( int hour, int min, int sec );
static void platform_rtc_getdate( int* day, int* month, int* year );
static void platform_rtc_setdate( int day, int month, int year );
static void platform_rtc_setalarmdate( int day, int month, int year );
static void platform_rtc_setalarmtime( int hour, int min, int sec );

#endif
