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
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.read
#define i2cWrite Wire.write
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
            TIME_REG        {0x00}, // 7 registers, Seconds, Minutes, Hours, DOW, Date, Month, Year
            DAY_REG         {0x03}, // the RTC Day register contains the OSCON, VBAT, and VBATEN bits
            YEAR_REG        {0x06}, // RTC year register
            CTRL_REG        {0x07}, // control register
            CALIB_REG       {0x08}, // calibration register
            UNLOCK_ID_REG   {0x09}, // unlock ID register
            ALM0_REG        {0x0A}, // alarm 0, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
            ALM1_REG        {0x11}, // alarm 1, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
            ALM0_DAY        {0x0D}, // DOW register has alarm config/flag bits
            PWRDWN_TS_REG   {0x18}, // power-down timestamp, 4 registers, Minutes, Hours, Date, Month
            PWRUP_TS_REG    {0x1C}, // power-up timestamp, 4 registers, Minutes, Hours, Date, Month
            TIMESTAMP_SIZE  {8},    // number of bytes in the two timestamp registers
            SRAM_START_ADDR {0x20}, // first SRAM address
            SRAM_SIZE       {64},   // number of bytes of SRAM
            EEPROM_SIZE     {128},  // number of bytes of EEPROM
            EEPROM_PAGE_SIZE{8},    // number of bytes on an EEPROM page
            UNIQUE_ID_ADDR  {0xF0}, // starting address for unique ID
            UNIQUE_ID_SIZE  {8};    // number of bytes in unique ID

        // Control Register bits
        static constexpr uint8_t
            OUT     {7},    // sets logic level on MFP when not used as square wave output
            SQWE    {6},    // set to enable square wave output
            ALM1    {5},    // alarm 1 is active
            ALM0    {4},    // alarm 0 is active
            EXTOSC  {3},    // set to drive the RTC registers from an external oscillator instead of a crystal
            RS2     {2},    // RS2:0 set square wave output frequency: 0==1Hz, 1==4096Hz, 2==8192Hz, 3=32768Hz
            RS1     {1},
            RS0     {0};

        // Other Control Bits
        static constexpr uint8_t
            ST      {7},    // Seconds register (TIME_REG) oscillator start/stop bit, 1==Start, 0==Stop
            HR1224  {6},    // Hours register (TIME_REG+2) 12 or 24 hour mode (24 hour mode==0)
            AMPM    {5},    // Hours register (TIME_REG+2) AM/PM bit for 12 hour mode
            OSCON   {5},    // Day register (TIME_REG+3) oscillator running (set and cleared by hardware)
            VBAT    {4},    // Day register (TIME_REG+3) set by hardware when Vcc fails and RTC runs on battery.
                            // VBAT is cleared by software, clearing VBAT also clears the timestamp registers
            VBATEN  {3},    // Day register (TIME_REG+3) VBATEN==1 enables backup battery, VBATEN==0 disconnects the VBAT pin (e.g. to save battery)
            LP      {5};    // Month register (TIME_REG+5) leap year bit

        // Alarm Control Bits
        static constexpr uint8_t
            ALMPOL  {7},     // Alarm Polarity: Defines the logic level for the MFP when an alarm is triggered.
            ALMC2   {6},     // Alarm configuration bits determine how alarms match. See ALM_MATCH defines below.
            ALMC1   {5},
            ALMC0   {4},
            ALMIF   {3};     // Alarm Interrupt Flag: Set by hardware when an alarm was triggered, cleared by software.

        MCP79412RTC() {};
        MCP79412RTC(bool initI2C) { (void)initI2C; }  // undocumented for backward compatibility
        void begin();
        static time_t get();
        static void set(time_t t);
        static bool read(tmElements_t &tm);
        static void write(tmElements_t &tm);
        void sramWrite(uint8_t addr, uint8_t value);
        void sramWrite(uint8_t addr, uint8_t* values, uint8_t nBytes);
        uint8_t sramRead(uint8_t addr);
        void sramRead(uint8_t addr, uint8_t* values, uint8_t nBytes);
        void eepromWrite(uint8_t addr, uint8_t value);
        void eepromWrite(uint8_t addr, uint8_t* values, uint8_t nBytes);
        uint8_t eepromRead(uint8_t addr);
        void eepromRead(uint8_t addr, uint8_t* values, uint8_t nBytes);
        int16_t calibRead();
        void calibWrite(int16_t value);
        void idRead(uint8_t* uniqueID);
        void getEUI64(uint8_t* uniqueID);
        bool powerFail(time_t* powerDown, time_t* powerUp);
        void squareWave(uint8_t freq);
        void setAlarm(ALARM_NBR_t alarmNumber, time_t alarmTime);
        void enableAlarm(ALARM_NBR_t alarmNumber, uint8_t alarmType);
        bool alarm(ALARM_NBR_t alarmNumber);
        void out(bool level);
        void alarmPolarity(bool polarity);
        bool isRunning();
        void vbaten(bool enable);
        void dumpRegs(uint32_t startAddr=0, uint32_t nBytes=32);
        void dumpSRAM(uint32_t startAddr=0, uint32_t nBytes=64);
        void dumpEEPROM(uint32_t startAddr=0, uint32_t nBytes=128);

    private:
        static void ramWrite(uint8_t addr, uint8_t value);
        static void ramWrite(uint8_t addr, uint8_t* values, uint8_t nBytes);
        static uint8_t ramRead(uint8_t addr);
        static void ramRead(uint8_t addr, uint8_t* values, uint8_t nBytes);
        static uint8_t eepromWait();
        static uint8_t dec2bcd(uint8_t num);
        static uint8_t bcd2dec(uint8_t num);
};

#endif
