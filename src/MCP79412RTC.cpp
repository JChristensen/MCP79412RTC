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

#include <MCP79412RTC.h>

// Initialize the I2C bus.
void MCP79412RTC::begin()
{
    i2cBegin();
}

// Read the current time from the RTC and return it as a time_t value.
// Returns a zero value if RTC not present (I2C I/O error).
time_t MCP79412RTC::get()
{
    tmElements_t tm;

    if ( read(tm) )
        return( makeTime(tm) );
    else
        return 0;
}

// Set the RTC to the given time_t value.
void MCP79412RTC::set(const time_t t)
{
    tmElements_t tm;

    breakTime(t, tm);
    write(tm);
}

// Read the current time from the RTC and return it in a tmElements_t
// structure. Returns false if RTC not present (I2C I/O error).
bool MCP79412RTC::read(tmElements_t& tm)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(RTCSEC);
    if (i2cEndTransmission() != 0) {
        return false;
    }
    else {
        // request 7 bytes (secs, min, hr, dow, date, mth, yr)
        i2cRequestFrom(RTC_ADDR, static_cast<uint8_t>(tmNbrFields));
        tm.Second = bcd2dec(i2cRead() & ~_BV(STOSC));
        tm.Minute = bcd2dec(i2cRead());
        tm.Hour = bcd2dec(i2cRead() & ~_BV(HR1224));    // assumes 24hr clock
        tm.Wday = i2cRead() & ~(_BV(OSCRUN) | _BV(PWRFAIL) | _BV(VBATEN));  // mask off OSCRUN, PWRFAIL, VBATEN bits
        tm.Day = bcd2dec(i2cRead());
        tm.Month = bcd2dec(i2cRead() & ~_BV(LPYR));     // mask off the leap year bit
        tm.Year = y2kYearToTm(bcd2dec(i2cRead()));
        return true;
    }
}

// Set the RTC's time from a tmElements_t structure.
void MCP79412RTC::write(const tmElements_t& tm)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(RTCSEC);
    i2cWrite(0x00);                             // stops the oscillator (Bit 7, STOSC == 0)
    i2cWrite(dec2bcd(tm.Minute));
    i2cWrite(dec2bcd(tm.Hour));                 // sets 24 hour format (Bit 6 == 0)
    i2cWrite(tm.Wday | _BV(VBATEN));            // enable battery backup operation
    i2cWrite(dec2bcd(tm.Day));
    i2cWrite(dec2bcd(tm.Month));
    i2cWrite(dec2bcd(tmYearToY2k(tm.Year)));
    i2cEndTransmission();

    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(RTCSEC);
    i2cWrite(dec2bcd(tm.Second) | _BV(STOSC));  // set the seconds and start the oscillator (Bit 7, STOSC == 1)
    i2cEndTransmission();
}

// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0x5F, no checking.
void MCP79412RTC::ramWrite(const uint8_t addr, const uint8_t value)
{
    ramWrite(addr, &value, 1);
}

// Write multiple bytes to RTC RAM.
// Valid address range is 0x00 - 0x5F, no checking.
// Number of bytes (nBytes) must be between 1 and 31 (Wire library
// limitation).
void MCP79412RTC::ramWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(addr);
    for (uint8_t i=0; i<nBytes; i++) i2cWrite(values[i]);
    i2cEndTransmission();
}

// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0x5F, no checking.
uint8_t MCP79412RTC::ramRead(uint8_t addr)
{
    uint8_t value;

    ramRead(addr, &value, 1);
    return value;
}

// Read multiple bytes from RTC RAM.
// Valid address range is 0x00 - 0x5F, no checking.
// Number of bytes (nBytes) must be between 1 and 32 (Wire library
// limitation).
void MCP79412RTC::ramRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes)
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(addr);
    i2cEndTransmission();
    i2cRequestFrom(RTC_ADDR, nBytes);
    for (uint8_t i=0; i<nBytes; i++) values[i] = i2cRead();
}

// Write a single byte to Static RAM.
// Address (addr) is constrained to the range (0, 63).
void MCP79412RTC::sramWrite(const uint8_t addr, const uint8_t value)
{
    ramWrite( (addr & (SRAM_SIZE - 1) ) + SRAM_START_ADDR, &value, 1 );
}

// Write multiple bytes to Static RAM.
// Address (addr) is constrained to the range (0, 63).
// Number of bytes (nBytes) must be between 1 and 31 (Wire library
// limitation).
// Invalid values for nBytes, or combinations of addr and nBytes
// that would result in addressing past the last byte of SRAM will
// result in no action.
void MCP79412RTC::sramWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes)
{
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    if (nBytes >= 1 && (addr + nBytes) <= SRAM_SIZE) {
#else
    if (nBytes >= 1 && nBytes <= (BUFFER_LENGTH - 1) && (addr + nBytes) <= SRAM_SIZE) {
#endif
        ramWrite( (addr & (SRAM_SIZE - 1) ) + SRAM_START_ADDR, values, nBytes );
    }
}

// Read a single byte from Static RAM.
// Address (addr) is constrained to the range (0, 63).
uint8_t MCP79412RTC::sramRead(const uint8_t addr)
{
    uint8_t value;

    ramRead( (addr & (SRAM_SIZE - 1) ) + SRAM_START_ADDR, &value, 1 );
    return value;
}

// Read multiple bytes from Static RAM.
// Address (addr) is constrained to the range (0, 63).
// Number of bytes (nBytes) must be between 1 and 32 (Wire library
// limitation).
// Invalid values for nBytes, or combinations of addr and
// nBytes that would result in addressing past the last byte of SRAM
// result in no action.
void MCP79412RTC::sramRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes)
{
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    if (nBytes >= 1 && (addr + nBytes) <= SRAM_SIZE) {
#else
    if (nBytes >= 1 && nBytes <= BUFFER_LENGTH && (addr + nBytes) <= SRAM_SIZE) {
#endif
        ramRead((addr & (SRAM_SIZE - 1) ) + SRAM_START_ADDR, values, nBytes);
    }
}

// Write a single byte to EEPROM.
// Address (addr) is constrained to the range (0, 127).
// Can't leverage page write function because a write can't start
// mid-page.
void MCP79412RTC::eepromWrite(const uint8_t addr, const uint8_t value)
{
    i2cBeginTransmission(EEPROM_ADDR);
    i2cWrite( addr & (EEPROM_SIZE - 1) );
    i2cWrite(value);
    i2cEndTransmission();
    eepromWait();
}

// Write a page (or less) to EEPROM. An EEPROM page is 8 bytes.
// Address (addr) should be a page start address (0, 8, ..., 120), but
// is ruthlessly coerced into a valid value.
// Number of bytes (nBytes) must be between 1 and 8, other values
// result in no action.
void MCP79412RTC::eepromWrite(const uint8_t addr, const uint8_t* values, const uint8_t nBytes)
{
    if (nBytes >= 1 && nBytes <= EEPROM_PAGE_SIZE) {
        i2cBeginTransmission(EEPROM_ADDR);
        i2cWrite( addr & ~(EEPROM_PAGE_SIZE - 1) & (EEPROM_SIZE - 1) );
        for (uint8_t i=0; i<nBytes; i++) i2cWrite(values[i]);
        i2cEndTransmission();
        eepromWait();
    }
}

// Read a single byte from EEPROM.
// Address (addr) is constrained to the range (0, 127).
uint8_t MCP79412RTC::eepromRead(const uint8_t addr)
{
    uint8_t value;

    eepromRead( addr & (EEPROM_SIZE - 1), &value, 1 );
    return value;
}

// Read multiple bytes from EEPROM.
// Address (addr) is constrained to the range (0, 127).
// Number of bytes (nBytes) must be between 1 and 32 (Wire library
// limitation).
// Invalid values for addr or nBytes, or combinations of addr and
// nBytes that would result in addressing past the last byte of EEPROM
// result in no action.
void MCP79412RTC::eepromRead(const uint8_t addr, uint8_t* values, const uint8_t nBytes)
{
#if defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    if (nBytes >= 1 && (addr + nBytes) <= EEPROM_SIZE) {
#else
    if (nBytes >= 1 && nBytes <= BUFFER_LENGTH && (addr + nBytes) <= EEPROM_SIZE) {
#endif
        i2cBeginTransmission(EEPROM_ADDR);
        i2cWrite( addr & (EEPROM_SIZE - 1) );
        i2cEndTransmission();
        i2cRequestFrom(EEPROM_ADDR, nBytes);
        for (uint8_t i=0; i<nBytes; i++) values[i] = i2cRead();
    }
}

// Wait for EEPROM write to complete.
uint8_t MCP79412RTC::eepromWait()
{
    uint8_t waitCount{0};
    uint8_t txStatus;

    do {
        ++waitCount;
        i2cBeginTransmission(EEPROM_ADDR);
        i2cWrite(0);
        txStatus = i2cEndTransmission();
    } while (txStatus != 0);

    return waitCount;
}

// Read the calibration register.
// The calibration value is not a twos-complement number. The MSB is
// the sign bit, and the 7 LSBs are an unsigned number, so we convert
// it and return it to the caller as a regular twos-complement integer.
int16_t MCP79412RTC::calibRead()
{
    uint8_t val {ramRead(OSCTRIM)};

    if ( val & 0x80 ) return -(val & 0x7F);
    else return val;
}

// Write the calibration register.
// Calibration value must be between -127 and 127, others result
// in no action. See note above on the format of the calibration value.
void MCP79412RTC::calibWrite(const int16_t value)
{
    if (value >= -127 && value <= 127) {
        uint8_t calibVal = abs(value);
        if (value < 0) calibVal += 128;
        ramWrite(OSCTRIM, calibVal);
    }
}

// Read the unique ID.
// For the MCP79411 (EUI-48), the first two bytes will contain 0xFF.
// Caller must provide an 8-byte array to contain the results.
void MCP79412RTC::idRead(uint8_t* uniqueID)
{
    i2cBeginTransmission(EEPROM_ADDR);
    i2cWrite(UNIQUE_ID_ADDR);
    i2cEndTransmission();
    i2cRequestFrom( EEPROM_ADDR, UNIQUE_ID_SIZE );
    for (uint8_t i=0; i<UNIQUE_ID_SIZE; i++) uniqueID[i] = i2cRead();
}

// Returns an EUI-64 ID. For an MCP79411, the EUI-48 ID is converted to
// EUI-64. For an MCP79412, calling this function is equivalent to
// calling idRead(). For an MCP79412, if the RTC type is known, calling
// idRead() will be a bit more efficient.
// Caller must provide an 8-byte array to contain the results.
void MCP79412RTC::getEUI64(uint8_t* uniqueID)
{
    uint8_t rtcID[8];

    idRead(rtcID);
    if (rtcID[0] == 0xFF && rtcID[1] == 0xFF) {
        rtcID[0] = rtcID[2];
        rtcID[1] = rtcID[3];
        rtcID[2] = rtcID[4];
        rtcID[3] = 0xFF;
        rtcID[4] = 0xFE;
    }
    for (uint8_t i=0; i<UNIQUE_ID_SIZE; i++) uniqueID[i] = rtcID[i];
}

// Check to see if a power failure has occurred. If so, returns TRUE
// as the function value, and returns the power down and power up
// timestamps. After returning the time stamps, the RTC's timestamp
// registers are cleared and the PWRFAIL bit which indicates a power
// failure is reset.
//
// Note that the power down and power up timestamp registers do not
// contain values for seconds or for the year. The returned time stamps
// will therefore contain the current year from the RTC. However, there
// is a chance that a power outage spans from one year to the next.
// If we find the power down timestamp to be later (larger) than the
// power up timestamp, we will assume this has happened, and
// subtract one year from the power down timestamp.
//
// Still, there is an assumption that the timestamps are being read
// in the same year as that when the power up occurred.
//
// Finally, note that once the RTC records a power outage, it must be
// cleared before another will be recorded.
bool MCP79412RTC::powerFail(time_t* powerDown, time_t* powerUp)
{
    uint8_t day, yr;                // copies of the RTC Day and Year registers
    ramRead(RTCWKDAY, &day, 1);
    ramRead(RTCYEAR, &yr, 1);
    yr = y2kYearToTm(bcd2dec(yr));
    if ( day & _BV(PWRFAIL) ) {
        i2cBeginTransmission(RTC_ADDR);
        i2cWrite(PWRDNMIN);
        i2cEndTransmission();

        i2cRequestFrom(RTC_ADDR, TIMESTAMP_SIZE);       // read both timestamp registers, 8 bytes total
        tmElements_t dn, up;                            // power down and power up times
        dn.Second = 0;
        dn.Minute = bcd2dec(i2cRead());
        dn.Hour = bcd2dec(i2cRead() & ~_BV(HR1224));    // assumes 24hr clock
        dn.Day = bcd2dec(i2cRead());
        dn.Month = bcd2dec(i2cRead() & 0x1F);           // mask off the day, we don't need it
        dn.Year = yr;                                   // assume current year
        up.Second = 0;
        up.Minute = bcd2dec(i2cRead());
        up.Hour = bcd2dec(i2cRead() & ~_BV(HR1224));    // assumes 24hr clock
        up.Day = bcd2dec(i2cRead());
        up.Month = bcd2dec(i2cRead() & 0x1F);           // mask off the day, we don't need it
        up.Year = yr;                                   // assume current year

        *powerDown = makeTime(dn);
        *powerUp = makeTime(up);

        // clear the PWRFAIL bit, which causes the RTC hardware to clear the timestamps too.
        // I suppose there is a risk here that the day has changed since we read it,
        // but the Day of Week is actually redundant data and the makeTime() function
        // does not use it. This could be an issue if someone is reading the RTC
        // registers directly, but as this library is meant to be used with the Time library,
        // and also because we don't provide a method to read the RTC clock/calendar
        // registers directly, we won't lose any sleep about it at this point unless
        // some issue is actually brought to our attention ;-)
        day &= ~_BV(PWRFAIL);
        ramWrite(RTCWKDAY, &day , 1);

        // adjust the powerDown timestamp if needed (see notes above)
        if (*powerDown > *powerUp) {
            --dn.Year;
            *powerDown = makeTime(dn);
        }
        return true;
    }
    else
        return false;
}

// Enable or disable the square wave output.
void MCP79412RTC::squareWave(const SQWAVE_FREQS_t freq)
{
    uint8_t ctrlReg;
    ramRead(CONTROL, &ctrlReg, 1);
    if (freq > 3) {
        ctrlReg &= ~_BV(SQWEN);
    }
    else {
        ctrlReg = (ctrlReg & 0xF8) | _BV(SQWEN) | freq;
    }
    ramWrite(CONTROL, &ctrlReg, 1);
}

// Set an alarm to the given time_t value. Sets the alarm registers only,
// does not enable the alarm. See enableAlarm().
void MCP79412RTC::setAlarm(const ALARM_NBR_t alarmNumber, const time_t alarmTime)
{
    uint8_t day;                // need to preserve bits in the day (of week) register
    ramRead( ALM0WKDAY + alarmNumber * (ALM1SEC - ALM0SEC), &day, 1);
    tmElements_t tm;
    breakTime(alarmTime, tm);
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite( ALM0SEC + alarmNumber * (ALM1SEC - ALM0SEC) );
    i2cWrite(dec2bcd(tm.Second));
    i2cWrite(dec2bcd(tm.Minute));
    i2cWrite(dec2bcd(tm.Hour));         // sets 24 hour format (Bit 6 == 0)
    i2cWrite( (day & 0xF8) + tm.Wday );
    i2cWrite(dec2bcd(tm.Day));
    i2cWrite(dec2bcd(tm.Month));
    i2cEndTransmission();
}

// Set an alarm by specifying year, month, day, hour, minute, second.
// While the RTC does not use year for alarms, it's important to use the
// correct year to ensure the day of the week is calculated correctly. This
// is important especially if a day of week alarm will be used.
// Use a four-digit year, including century e.g. CCYY.
void MCP79412RTC::setAlarm(const ALARM_NBR_t alarmNumber, const uint16_t y, const uint8_t mon,
                  const uint8_t d, const uint8_t h, const uint8_t m, const uint8_t s)
{
    tmElements_t tm;
    tm.Year = CalendarYrToTm(y);
    tm.Month = mon;
    tm.Day = d;
    tm.Hour = h;
    tm.Minute = m;
    tm.Second = s;
    setAlarm(alarmNumber, makeTime(tm));
}

// Enable or disable an alarm, and set the trigger criteria,
// e.g. match only seconds, only minutes, entire time and date, etc.
void MCP79412RTC::enableAlarm(const ALARM_NBR_t alarmNumber, const ALARM_TYPES_t alarmType)
{
    uint8_t ctrl;               // control register has alarm enable bits
    ramRead(CONTROL, &ctrl, 1);
    if (alarmType < ALM_DISABLE) {
        uint8_t day;                            // alarm day register has config & flag bits
        ramRead(ALM0WKDAY + alarmNumber * (ALM1SEC - ALM0SEC), &day, 1);
        day = ( day & 0x87 ) | alarmType << 4;  // reset interrupt flag, OR in the config bits
        ramWrite(ALM0WKDAY + alarmNumber * (ALM1SEC - ALM0SEC), &day, 1);
        ctrl |= _BV(ALM0EN + alarmNumber);      // enable the alarm
    }
    else {
        ctrl &= ~(_BV(ALM0EN + alarmNumber));   // disable the alarm
    }
    ramWrite(CONTROL, &ctrl, 1);
}

// Returns true or false depending on whether the given alarm has been
// triggered, and resets the alarm "interrupt" flag. This is not a real
// interrupt, just a bit that's set when an alarm is triggered.
bool MCP79412RTC::alarm(const ALARM_NBR_t alarmNumber)
{
    uint8_t day;                // alarm day register has config & flag bits
    ramRead( ALM0WKDAY + alarmNumber * (ALM1SEC - ALM0SEC), &day, 1);
    if (day & _BV(ALMxIF)) {
        day &= ~_BV(ALMxIF);    // turn off the alarm "interrupt" flag
        ramWrite( ALM0WKDAY + alarmNumber * (ALM1SEC - ALM0SEC), &day, 1);
        return true;
    }
    else
        return false;
}

// Sets the logic level on the MFP when it's not being used as a
// square wave or alarm output. The default is HIGH.
void MCP79412RTC::out(const bool level)
{
    uint8_t ctrlReg;
    ramRead(CONTROL, &ctrlReg, 1);
    if (level)
        ctrlReg |= _BV(OUT);
    else
        ctrlReg &= ~_BV(OUT);
    ramWrite(CONTROL, &ctrlReg, 1);
}

// Specifies the logic level on the Multi-Function Pin (MFP) when an
// alarm is triggered. The default is LOW. When both alarms are
// active, the two are ORed together to determine the level of the MFP.
// With alarm polarity set to LOW (the default), this causes the MFP
// to go low only when BOTH alarms are triggered. With alarm polarity
// set to HIGH, the MFP will go high when EITHER alarm is triggered.
//
// Note that the state of the MFP is independent of the alarm
// "interrupt" flags, and the alarm() function will indicate when an
// alarm is triggered regardless of the polarity.
void MCP79412RTC::alarmPolarity(const bool polarity)
{
    uint8_t alm0Day;
    ramRead(ALM0WKDAY, &alm0Day, 1);
    if (polarity)
        alm0Day |= _BV(ALMPOL);
    else
        alm0Day &= ~_BV(ALMPOL);
    ramWrite(ALM0WKDAY, &alm0Day, 1);
}

// Check to see if the RTC's oscillator is started (STOSC bit in seconds
// register). Returns true if started.
bool MCP79412RTC::isRunning()
{
    i2cBeginTransmission(RTC_ADDR);
    i2cWrite(RTCSEC);
    i2cEndTransmission();

    // request just the seconds register
    i2cRequestFrom(RTC_ADDR, static_cast<uint8_t>(1));
    return i2cRead() & _BV(STOSC);
}

// Set or clear the VBATEN bit. Setting the bit powers the clock and
// SRAM from the backup battery when Vcc falls. Note that setting the
// time via set() or write() sets the VBATEN bit.
void MCP79412RTC::vbaten(const bool enable)
{
    uint8_t day;
    ramRead(RTCWKDAY, &day, 1);
    if (enable)
        day |= _BV(VBATEN);
    else
        day &= ~_BV(VBATEN);

    ramWrite(RTCWKDAY, &day, 1);
    return;
}

// Decimal-to-BCD conversion
uint8_t MCP79412RTC::dec2bcd(const uint8_t n)
{
    return n + 6 * (n / 10);
}

// BCD-to-Decimal conversion
uint8_t __attribute__ ((noinline)) MCP79412RTC::bcd2dec(const uint8_t n)
{
    return n - 6 * (n >> 4);
}

// dump rtc registers, 16 bytes at a time.
// always dumps a multiple of 16 bytes.
// duplicate rows are suppressed and indicated with an asterisk.
void MCP79412RTC::dumpRegs(const uint32_t startAddr, const uint32_t nBytes)
{
    Serial.print(F("\nRTC REGISTERS\n"));
    uint32_t nRows = (nBytes + 15) >> 4;

    uint8_t d[16], last[16];
    uint32_t aLast {startAddr};
    for (uint32_t r = 0; r < nRows; r++) {
        uint32_t a = startAddr + 16 * r;
        ramRead(a, d, 16);
        bool same {true};
        for (int i=0; i<16; ++i) {
            if (last[i] != d[i]) same = false;
        }
        if (!same || r == 0 || r == nRows-1) {
            Serial.print(F("0x"));
            if ( a < 16 * 16 * 16 ) Serial.print('0');
            if ( a < 16 * 16 ) Serial.print('0');
            if ( a < 16 ) Serial.print('0');
            Serial.print(a, HEX);
            Serial.print(a == aLast+16 || r == 0 ? "  " : "* ");
            for ( int16_t c = 0; c < 16; c++ ) {
                if ( d[c] < 16 ) Serial.print('0');
                Serial.print(d[c], HEX);
                Serial.print(c == 7 ? "  " : " " );
            }
            Serial.println();
            aLast = a;
        }
        for (int i=0; i<16; ++i) {
            last[i] = d[i];
        }
    }
}

// dump rtc sram, 16 bytes at a time.
// always dumps a multiple of 16 bytes.
// duplicate rows are suppressed and indicated with an asterisk.
void MCP79412RTC::dumpSRAM(const uint32_t startAddr, const uint32_t nBytes)
{
    Serial.print(F("\nRTC SRAM\n"));
    uint32_t nRows = (nBytes + 15) >> 4;

    uint8_t d[16], last[16];
    uint32_t aLast {startAddr};
    for (uint32_t r = 0; r < nRows; r++) {
        uint32_t a = startAddr + 16 * r;
        sramRead(a, d, 16);
        bool same {true};
        for (int i=0; i<16; ++i) {
            if (last[i] != d[i]) same = false;
        }
        if (!same || r == 0 || r == nRows-1) {
            Serial.print(F("0x"));
            if ( a < 16 * 16 * 16 ) Serial.print('0');
            if ( a < 16 * 16 ) Serial.print('0');
            if ( a < 16 ) Serial.print('0');
            Serial.print(a, HEX);
            Serial.print(a == aLast+16 || r == 0 ? "  " : "* ");
            for ( int16_t c = 0; c < 16; c++ ) {
                if ( d[c] < 16 ) Serial.print('0');
                Serial.print(d[c], HEX);
                Serial.print(c == 7 ? "  " : " " );
            }
            Serial.println();
            aLast = a;
        }
        for (int i=0; i<16; ++i) {
            last[i] = d[i];
        }
    }
}

// dump rtc eeprom, 16 bytes at a time.
// always dumps a multiple of 16 bytes.
// duplicate rows are suppressed and indicated with an asterisk.
void MCP79412RTC::dumpEEPROM(const uint32_t startAddr, const uint32_t nBytes)
{
    Serial.print(F("\nRTC EEPROM\n"));
    uint32_t nRows = (nBytes + 15) >> 4;

    uint8_t d[16], last[16];
    uint32_t aLast {startAddr};
    for (uint32_t r = 0; r < nRows; r++) {
        uint32_t a = startAddr + 16 * r;
        eepromRead(a, d, 16);
        bool same {true};
        for (int i=0; i<16; ++i) {
            if (last[i] != d[i]) same = false;
        }
        if (!same || r == 0 || r == nRows-1) {
            Serial.print(F("0x"));
            if ( a < 16 * 16 * 16 ) Serial.print('0');
            if ( a < 16 * 16 ) Serial.print('0');
            if ( a < 16 ) Serial.print('0');
            Serial.print(a, HEX);
            Serial.print(a == aLast+16 || r == 0 ? "  " : "* ");
            for ( int16_t c = 0; c < 16; c++ ) {
                if ( d[c] < 16 ) Serial.print('0');
                Serial.print(d[c], HEX);
                Serial.print(c == 7 ? "  " : " " );
            }
            Serial.println();
            aLast = a;
        }
        for (int i=0; i<16; ++i) {
            last[i] = d[i];
        }
    }
}
