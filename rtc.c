#include "types.h"
#include "hwdefs.h"
#include "gbc_state.h"
#include "rtc.h"

/**
 * Starts or stops the RTC when the halt flag is updated.
 * Assumes only called on transition.
 * @param s Full GameBoy state.
 * @param isStopped true if Halting the timer, 0 is turning it on again.
  */
void toggleRealTimeClock(GbState* s, bool isStopped) {
    if (isStopped) {
        // Keep track of when we turn off the timer.
        s->Cartridge->RTCStopTime = get_ticks_ms();
    } else {
        // When we switch the timer back on, find out how much time elapsed since we turned it off.
        s->Cartridge->RTCTimeStopped += (get_ticks_ms() - s->Cartridge->RTCStopTime);
    }
}

/**
 * Updates the current RTC value based on the initial value and get_ticks_ms()
 * @returns Error Code
 */
sByte updateRealTimeClock(GbState* s) {
    if (s->Cartridge->Ram.Size < SRAM_BANK_SIZE * 0xD) {
        return LOAD_ERR_RTC_UNAVAILABLE;
    }

    RealTimeClockData rtcData;
    rtcData.Seconds = s->Cartridge->Ram.Data[SRAM_BANK_SIZE * 0x08];
    rtcData.Minutes = s->Cartridge->Ram.Data[SRAM_BANK_SIZE * 0x09];
    rtcData.Hours = s->Cartridge->Ram.Data[SRAM_BANK_SIZE * 0x0A];
    rtcData.DaysLow = s->Cartridge->Ram.Data[SRAM_BANK_SIZE * 0x0B];
    rtcData.StatusByte = s->Cartridge->Ram.Data[SRAM_BANK_SIZE * 0x0C];  

    long now = get_ticks_ms();
    long diff = now - s->Cartridge->LastRTCTicks + s->Cartridge->RTCTimeStopped;
    diff /= 1000;
    long seconds = (rtcData.Seconds + diff);
    rtcData.Seconds = seconds % 60;
    long minutes = rtcData.Minutes + (seconds / 60);
    rtcData.Minutes = minutes % 60;
    long hours = rtcData.Hours + (rtcData.Minutes / 60) + (rtcData.Seconds / 3600);
    rtcData.Hours = hours % 24;
    long days = rtcData.DaysLow + (rtcData.Hours / 24) + (rtcData.Minutes / 1440) + (rtcData.Seconds / 86400);
    rtcData.DaysLow = ((byte) days) & 0x7F;

    // TODO - These two sets are wrong, but I can't concentrate right now.
    if (days > 0xFF) {
        if (rtcData.Status.DaysHigh == 1) {
            // Carry flag stays set until reset directly.
            rtcData.Status.HasDayCarried = 1;
            // DaysHigh overflowed back to 0.
            rtcData.Status.DaysHigh = 0
        } else {
            rtcData.Status.DaysHigh = 1;
        }
    }

    memset(s->SRAM + (SRAM_BANK_SIZE * 0x08), rtcData.Seconds, SRAM_BANK_SIZE);
    memset(s->SRAM + (SRAM_BANK_SIZE * 0x09), rtcData.Minutes, SRAM_BANK_SIZE);
    memset(s->SRAM + (SRAM_BANK_SIZE * 0x0A), rtcData.Hours, SRAM_BANK_SIZE);
    memset(s->SRAM + (SRAM_BANK_SIZE * 0x0B), rtcData.DaysLow, SRAM_BANK_SIZE);
    memset(s->SRAM + (SRAM_BANK_SIZE * 0x0B), rtcData.StatusByte, SRAM_BANK_SIZE);

    s->Cartridge->LastRTCTicks = now;
    s->Cartridge->RTCTimeStopped = 0;    

    return 0;
}