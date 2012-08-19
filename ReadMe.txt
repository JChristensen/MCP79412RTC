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

Alpha Version 0.8 Aug-2012

Currently implemented:
Functions to set and read the time from the RTC.
Functions to read and write SRAM and EEPROM.
Functions to read and write the calibration register and
to read the 64-bit ID.

Not yet implemented:
Alarm functions: Set, read, enable, detect, etc.
Functions to read and reset the power fail timestamps.
Function(s) to enable and set the CLKOUT frequency.
Decent documentation.
Example sketches.

Brief descriptions of currently implemented functions follow.

Read the current time from the RTC and return as a time_t value.
time_t get(void);

Set the RTC to the given time_t value.
void set(time_t t);

Read the current time from the RTC and return as a tmElements_t structure.
See the Arduino Time library for details on the tmElements_t structure:
http://www.arduino.cc/playground/Code/Time.
read(tmElements_t &tm);

Set the RTC to the time represented in the given tmElements_t structure.
void write(tmElements_t &tm);

Write a single byte to the RTC static RAM.
void sramWrite(byte addr, byte value);

Write multiple bytes to the RTC static RAM. nBytes <= 31.
values is a byte array, e.g. byte myValues[31];
void sramWrite(byte addr, byte *values, byte nBytes);

Read a single byte from the RTC static RAM.
byte sramRead(byte addr);

Read multiple bytes from the RTC static RAM, place their values into the given array.
nBytes <= 31.
void sramRead(byte addr, byte *values, byte nBytes);

Write a single byte to the RTC EEPROM.
void eepromWrite(byte addr, byte value);

Write multiple bytes to the RTC EEPROM. nBytes <= 31.
values is a byte array, e.g. byte myValues[31];
void eepromWrite(byte addr, byte *values, byte nBytes);

Read a single byte from the RTC EEPROM.
byte eepromRead(byte addr);

Read multiple bytes from the RTC static RAM, place their values into the given array.
nBytes <= 31.
void eepromRead(byte addr, byte *values, byte nBytes);

Returns the value of the RTC calibration register.
This is an adjustment factor in PPM, and must be between -127 and 127.
int calibRead(void);

Write the given value into the RTC calibration register.
void calibWrite(int value);

Read the 64-bit unique ID from the RTC into the given 8-byte array.
void idRead(byte *uniqueID);
