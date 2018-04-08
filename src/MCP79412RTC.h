// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Arduino library for the Microchip MCP7941x Real-Time Clocks.
// Requires PJRC's improved version of the Arduino Time Library,
// https://playground.arduino.cc/Code/Time
// https://github.com/PaulStoffregen/Time
//
// For AVR architecture, an MCP79412RTC object named RTC is instantiated
// by the library and I2C initialization occurs in the constructor;
// this is for backwards compatibility.
// For other architectures, the user needs to instantiate a MCP79412RTC
// object and optionally initialize the I2C bus by calling
// MCP79412RTC::begin(). The constructor has an optional bool parameter
// to indicate whether I2C initialization should occur in the
// constructor; this parameter defaults to true if not given.

#ifndef MCP79412RTC_H_INCLUDED
#define MCP79412RTC_H_INCLUDED

#include <Arduino.h>
#include <TimeLib.h>    // https://github.com/PaulStoffregen/Time

// Alarm types for use with the enableAlarm() function
enum {
    ALM_MATCH_SECONDS,
    ALM_MATCH_MINUTES,
    ALM_MATCH_HOURS,
    ALM_MATCH_DAY,      // triggers alarm at midnight
    ALM_MATCH_DATE,
    ALM_RESERVED_5,     // do not use
    ALM_RESERVED_6,     // do not use
    ALM_MATCH_DATETIME,
    ALM_DISABLE
};

// Square-wave output frequencies for use with squareWave() function
enum {
    SQWAVE_1_HZ,
    SQWAVE_4096_HZ,
    SQWAVE_8192_HZ,
    SQWAVE_32768_HZ,
    SQWAVE_NONE
};

// constants for use with alarm functions
#define ALARM_0 0
#define ALARM_1 1

class MCP79412RTC
{
    public:
        MCP79412RTC(bool initI2C = true);
        void begin();
        static time_t get();
        static void set(time_t t);
        static bool read(tmElements_t &tm);
        static void write(tmElements_t &tm);
        void sramWrite(byte addr, byte value);
        void sramWrite(byte addr, byte *values, byte nBytes);
        byte sramRead(byte addr);
        void sramRead(byte addr, byte *values, byte nBytes);
        void eepromWrite(byte addr, byte value);
        void eepromWrite(byte addr, byte *values, byte nBytes);
        byte eepromRead(byte addr);
        void eepromRead(byte addr, byte *values, byte nBytes);
        int calibRead();
        void calibWrite(int value);
        void idRead(byte *uniqueID);
        void getEUI64(byte *uniqueID);
        bool powerFail(time_t *powerDown, time_t *powerUp);
        void squareWave(uint8_t freq);
        void setAlarm(uint8_t alarmNumber, time_t alarmTime);
        void enableAlarm(uint8_t alarmNumber, uint8_t alarmType);
        bool alarm(uint8_t alarmNumber);
        void out(bool level);
        void alarmPolarity(bool polarity);
        bool isRunning();
        void vbaten(bool enable);

    private:
        static void ramWrite(byte addr, byte value);
        static void ramWrite(byte addr, byte *values, byte nBytes);
        static byte ramRead(byte addr);
        static void ramRead(byte addr, byte *values, byte nBytes);
        static byte eepromWait();
        static uint8_t dec2bcd(uint8_t num);
        static uint8_t bcd2dec(uint8_t num);
};

#if defined ARDUINO_ARCH_AVR
extern MCP79412RTC RTC;
#endif

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#ifndef BUFFER_LENGTH       // a horrible and limiting kludge for samd (arduino zero)
#define BUFFER_LENGTH 32
#endif

#endif
