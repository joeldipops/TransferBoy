#ifndef RTC_INCLUDED
#define RTC_INCLUDED 1

#include "types.h"

typedef struct {
    byte HasDayCarried:1;
    byte IsTimerStopped:1;
    byte pad:5;
    byte DaysHigh:1;
} RealTimeClockStatus;

typedef struct {
    byte Seconds;
    byte Minutes;
    byte Hours;
    byte DaysLow;
    union {
        byte StatusByte;  
        RealTimeClockStatus Status;
    };
} RealTimeClockData;

/**
 * Initialises memory used by real time clock.
 * @param s Full GameBoy state.
 * @returns Error Code
 */
sByte updateRealTimeClock(GbState* s);

/**
 * Starts or stops the RTC when the halt flag is updated.
 * @param s Full GameBoy state.
 * @param isStopped true if Halting the timer, 0 is turning it on again.
 */
void toggleRealTimeClock(GbState* s, bool isStopped);

#endif