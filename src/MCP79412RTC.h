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

// define consistent I2C functions
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#include <TinyWireM.h>
#define i2cBegin TinyWireM.begin
#define i2cBeginTransmission TinyWireM.beginTransmission
#define i2cEndTransmission TinyWireM.endTransmission
#define i2cRequestFrom TinyWireM.requestFrom
#define i2cRead TinyWireM.receive
#define i2cWrite TinyWireM.send
#else
#include <Wire.h>
#define i2cBegin wire.begin
#define i2cBeginTransmission wire.beginTransmission
#define i2cEndTransmission wire.endTransmission
#define i2cRequestFrom wire.requestFrom
#define i2cRead wire.read
#define i2cWrite wire.write
#endif

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

#ifndef BUFFER_LENGTH       // a horrible and limiting kludge for samd (arduino zero)
#define BUFFER_LENGTH 32
#endif

class MCP79412RTC
{
    public:
        // Alarm types for use with the enableAlarm() function
        enum ALARM_TYPES_t {
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
        enum SQWAVE_FREQS_t {
            SQWAVE_1_HZ,
            SQWAVE_4096_HZ,
            SQWAVE_8192_HZ,
            SQWAVE_32768_HZ,
            SQWAVE_NONE
        };

        // constants for use with alarm functions
        enum ALARM_NBR_t {
            ALARM_0,
            ALARM_1
        };

        // MCP7941x I2C Addresses
        static constexpr uint8_t
            RTC_ADDR    {0x6F},
            EEPROM_ADDR {0x57};

        // MCP7941x Register Addresses
        static constexpr uint8_t
            RTCSEC          {0x00}, // 7 registers, Seconds, Minutes, Hours, DOW, Date, Month, Year
            RTCWKDAY        {0x03}, // the RTC Day register contains the OSCRUN, PWRFAIL, and VBATEN bits
            RTCYEAR         {0x06}, // RTC year register
            CONTROL         {0x07}, // control register
            OSCTRIM         {0x08}, // oscillator calibration register
            EEUNLOCK        {0x09}, // protected eeprom unlock register
            ALM0SEC         {0x0A}, // alarm 0, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
            ALM1SEC         {0x11}, // alarm 1, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
            ALM0WKDAY       {0x0D}, // DOW register has alarm config/flag bits
            PWRDNMIN        {0x18}, // power-down timestamp, 4 registers, Minutes, Hours, Date, Month
            PWRUPMIN        {0x1C}, // power-up timestamp, 4 registers, Minutes, Hours, Date, Month
            TIMESTAMP_SIZE  {8},    // number of bytes in the two timestamp registers
            SRAM_START_ADDR {0x20}, // first SRAM address
            SRAM_SIZE       {64},   // number of bytes of SRAM
            EEPROM_SIZE     {128},  // number of bytes of EEPROM
            EEPROM_PAGE_SIZE{8},    // number of bytes on an EEPROM page
            UNIQUE_ID_ADDR  {0xF0}, // starting address for unique ID in EEPROM
            UNIQUE_ID_SIZE  {8};    // number of bytes in unique ID

        // Control Register bits
        static constexpr uint8_t
            OUT     {7},    // sets logic level on MFP when not used as square wave output
            SQWEN   {6},    // set to enable square wave output
            ALM1EN  {5},    // alarm 1 is active
            ALM0EN  {4},    // alarm 0 is active
            EXTOSC  {3},    // enable external oscillator instead of a crystal
            CRSTRIM {2},    // coarse trim mode enable
            SQWFS1  {1},    // SQWFS1:0 square wave output freq: 0==1Hz, 1==4096Hz, 2==8192Hz, 3=32768Hz
            SQWFS0  {0};

        // Other Control Bits
        static constexpr uint8_t
            STOSC   {7},    // Seconds register (RTCSEC) oscillator start/stop bit, 1==Start, 0==Stop
            HR1224  {6},    // Hours register (RTCHOUR) 12 or 24 hour mode (24 hour mode==0)
            AMPM    {5},    // Hours register (RTCHOUR) AM/PM bit for 12 hour mode
            OSCRUN  {5},    // Day register (RTCWKDAY) oscillator running (set and cleared by hardware)
            PWRFAIL {4},    // Day register (RTCWKDAY) set by hardware when Vcc fails and RTC runs on battery.
                            // PWRFAIL is cleared by software, clearing PWRFAIL also
                            // clears the timestamp registers
            VBATEN  {3},    // Day register (RTCWKDAY) VBATEN==1 enables backup
                            // battery, VBATEN==0 disconnects the VBAT pin (e.g. to save battery)
            LPYR    {5};    // Month register (RTCMTH) leap year bit

        // Alarm Control Bits
        static constexpr uint8_t
            ALMPOL      {7},    // Alarm Polarity: Defines the logic level for the MFP when an alarm is triggered.
            ALMxMSK2    {6},    // Alarm configuration bits determine how alarms match. See ALARM_TYPES_t enum.
            ALMxMSK1    {5},
            ALMxMSK0    {4},
            ALMxIF      {3};    // Alarm Interrupt Flag: Set by hardware when an alarm was triggered, cleared by software.

        MCP79412RTC(TwoWire& tw=Wire) : wire(tw) {};
        void begin();
        time_t get();
        void set(const time_t t);
        bool read(tmElements_t& tm);
        void write(const tmElements_t& tm);
        void sramWrite(const uint8_t addr, const uint8_t value);
        void sramWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes);
        uint8_t sramRead(const uint8_t addr);
        void sramRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes);
        void eepromWrite(const uint8_t addr, const uint8_t value);
        void eepromWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes);
        uint8_t eepromRead(const uint8_t addr);
        void eepromRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes);
        int16_t calibRead();
        void calibWrite(const int16_t value);
        void idRead(uint8_t* uniqueID);
        void getEUI64(uint8_t* uniqueID);
        bool powerFail(time_t* powerDown, time_t* powerUp);
        void squareWave(const SQWAVE_FREQS_t freq);
        void setAlarm(const ALARM_NBR_t alarmNumber, const time_t alarmTime);
        void setAlarm(const ALARM_NBR_t alarmNumber, const uint16_t y, const uint8_t mon,
                      const uint8_t d, const uint8_t h, const uint8_t m, const uint8_t s);
        void enableAlarm(const ALARM_NBR_t alarmNumber, const ALARM_TYPES_t alarmType);
        bool alarm(const ALARM_NBR_t alarmNumber);
        void out(const bool level);
        void alarmPolarity(const bool polarity);
        bool isRunning();
        void vbaten(const bool enable);
        void dumpRegs(const uint32_t startAddr=0, const uint32_t nBytes=32);
        void dumpSRAM(const uint32_t startAddr=0, const uint32_t nBytes=64);
        void dumpEEPROM(const uint32_t startAddr=0, const uint32_t nBytes=128);

    private:
        TwoWire& wire;      // reference to Wire, Wire1, etc.
        void ramWrite(const uint8_t addr, const uint8_t value);
        void ramWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes);
        uint8_t ramRead(uint8_t addr);
        void ramRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes);
        uint8_t eepromWait();
        uint8_t dec2bcd(const uint8_t num);
        uint8_t bcd2dec(const uint8_t num);
};

#endif
