/*----------------------------------------------------------------------*
 * MCP79412RTC.cpp - Arduino library to support the Microchip MCP79412  *
 * Real-Time Clock. This library is intended for use with the Arduino   *
 * Time.h library, http://www.arduino.cc/playground/Code/Time           *
 *                                                                      *
 * This library is a drop-in replacement for the DS1307RTC.h library    *
 * by Michael Margolis that is supplied with the Arduino Time library   *
 * above. To change from using a DS1307 RTC to an MCP79412 RTC, it is   *
 * only necessary to change the #include statement to include           *
 * MCP79412RTC.h instead of DS1307RTC.h.                                *
 *                                                                      *
 * In addition, this library implements functions to support the        *
 * additional features of the MCP79412.                                 *
 *                                                                      *
 * Jack Christensen 29Jul2012                                           *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/ 

Alpha Version 0.9 Aug-2012

Currently implemented:
Functions to set and read the time from the RTC.
Functions to read and write SRAM and EEPROM.
Functions to read and write the calibration register and
to read the 64-bit ID.
Alarm functions: Set, enable, detect.
Functions to read and reset the power fail timestamps.
Function(s) to enable and set the CLKOUT frequency.
Function to set the MFP logic level when not used as a
square wave output.
Function to set alarm polarity.

Not yet implemented:
Decent documentation.
Example sketches.

Brief descriptions of currently implemented functions follow.

time_t get(void);
Read the current time from the RTC and return as a time_t value.

void set(time_t t);
Set the RTC to the given time_t value.

read(tmElements_t &tm);
Read the current time from the RTC and return as a tmElements_t structure.
See the Arduino Time library for details on the tmElements_t structure:
http://www.arduino.cc/playground/Code/Time.

void write(tmElements_t &tm);
Set the RTC to the time represented in the given tmElements_t structure.

void sramWrite(byte addr, byte value);
Write a single byte to the RTC static RAM.

void sramWrite(byte addr, byte *values, byte nBytes);
Write multiple bytes to the RTC static RAM. nBytes <= 31.
values is a byte array, e.g. byte myValues[31];

byte sramRead(byte addr);
Read a single byte from the RTC static RAM.

void sramRead(byte addr, byte *values, byte nBytes);
Read multiple bytes from the RTC static RAM, place their values into the given array.
nBytes <= 31.

void eepromWrite(byte addr, byte value);
Write a single byte to the RTC EEPROM.

void eepromWrite(byte addr, byte *values, byte nBytes);
Write multiple bytes to the RTC EEPROM. nBytes <= 31.
values is a byte array, e.g. byte myValues[31];

byte eepromRead(byte addr);
Read a single byte from the RTC EEPROM.

void eepromRead(byte addr, byte *values, byte nBytes);
Read multiple bytes from the RTC static RAM, place their values into the given array.
nBytes <= 31.

int calibRead(void);
Returns the value of the RTC calibration register.
This is an adjustment factor in PPM, and must be between -127 and 127.

void calibWrite(int value);
Write the given value into the RTC calibration register.

void idRead(byte *uniqueID);
Read the 64-bit unique ID from the RTC into the given 8-byte array.

boolean powerFail(time_t *powerDown, time_t *powerUp);
Returns true or false to indicate whether a power failure has occurred.
If one occurred, the power down and power up timestamps are returned in the variables
given by the caller, the RTC's power fail flag is reset and the power up/down
timestamps are cleared.

void squareWave(uint8_t freq);
Enables or disables the square wave output on the multi-function pin (MFP).
freq is one of the following:
SQWAVE_1_HZ, SQWAVE_4096_HZ, SQWAVE_8192_HZ, SQWAVE_32768_HZ, SQWAVE_NONE

void setAlarm(uint8_t alarmNumber, time_t alarmTime);
Set an alarm time. Sets the alarm registers only, does not enable
the alarm, use enableAlarm() for that. alarmNumber is 0 or 1, but is
ruthlessly masked to ensure this is so.

void enableAlarm(uint8_t alarmNumber, uint8_t alarmType);
Enable or disable the given alarm (0 or 1).
alarmNumber is masked to ensure a value of 0 or 1.
alarmType is one of the following:
ALM_MATCH_SECONDS, ALM_MATCH_MINUTES, ALM_MATCH_HOURS, ALM_MATCH_DAY, ALM_MATCH_DATE, ALM_MATCH_DATETIME, ALM_DISABLE

boolean alarm(uint8_t alarmNumber);
Tests whether the given alarm (0 or 1) has been triggered, returns
true if triggered, else false. Clears the alarm flag to trap the next
trigger event can be trapped.

void out(boolean level);
Sets the logic level on the MFP when it's not being used as a
square wave or alarm output. The default is HIGH.

void alarmPolarity(boolean polarity);
Specifies the logic level on the Multi-Function Pin (MFP) when an
alarm is triggered. The default is HIGH. When both alarms are
active, the two are ORed together to determine the level of the MFP.
With alarm polarity set to LOW (the default), this causes the MFP
to go low only when BOTH alarms are triggered. With alarm polarity
set to HIGH, the MFP will go high when EITHER alarm is triggered.
Note that the state of the MFP is independent of the alarm
"interrupt" flags, and the alarm() function will indicate when an
alarm is triggered regardless of the polarity.
