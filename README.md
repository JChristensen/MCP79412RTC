# Arduino MCP79412 RTC Library
https://github.com/JChristensen/MCP79412RTC  
README file  
Jack Christensen  
May 2025

## License
Arduino MCP79412 RTC Library Copyright (C) 2012-2025 byJack Christensen GNU GPL v3.0

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License v3.0 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/gpl.html>

## Version 3.0.0 notes
Version 3.0.0 adds the ability to address different I2C peripherals for microcontrollers that have more than one (e.g. `Wire`, `Wire1`, etc.) There is one situation that is not backwards compatible with earlier versions of the library. Code that calls `setSyncProvider()` in the [Time library](https://github.com/PaulStoffregen/Time), e.g.:
```c++
    setSyncProvider(myRTC.get);
```
Needs to change as follows:
```c++
    setSyncProvider([](){return myRTC.get();});
```
This uses a lambda to provide the Time library with a static function, which it requires. The `MCP79412::get()` function was static in previous versions of the library, but no longer is in v3.0.0. This allows the flexibility to operate with different I2C peripherals.

## Version 2.0.0 notes
The 2.0.0 version of the library has some significant changes and is not completely backwards compatible with earlier versions. These changes provide a more consistent API and reduce the possibility of name collisions. While sketches using this library will likely require changes as a result, these should be mostly straightforward.

 - The library no longer defines an `MCP79412RTC` object, therefore each sketch needs to define  one. (Previous versions of the library defined an `MCP79412RTC` object named `RTC`, although only for AVR architecture. Consider using a name other than `RTC` as this can cause a name collision on some architectures.)
 - The constructor no longer has the capability to initialize the I2C bus and no longer accepts an optional parameter. Therefore, the sketch needs to call `MCP79412RTC::begin()` in the `setup()` function or elsewhere as appropriate.
 - To reduce the possibility of name collisions, the enumerations as well as register addresses, etc. are now defined in the header file within the `MCP79412RTC` class. Therefore, when using any of these names, it will be necessary to include the `MCP79412RTC` scope, e.g. `myRTC.alarm(MCP79412RTC::ALARM_1);`
 - The example sketches and documentation have been updated to reflect these changes.

## Introduction
**MCP79412RTC** is an Arduino library that supports the Microchip MCP7941x Real-Time Clock/Calendar chips. This library is intended to be used with [PJRC's Arduino Time library](https://github.com/PaulStoffregen/Time).

**For more information on the MCP79412, see:**  
The [Microchip MCP79412 Product Page](https://www.microchip.com/en-us/product/MCP79412) for specs, datasheet, etc.  
[My Blog Post](http://goo.gl/MkBnjR), summarizing the features and advantages of the MCP79412  
[My Power Outage Logger Project](http://goo.gl/RfM5os), an Arduino-based project featuring the MCP79412  

## Examples
The following example sketches are included with the **MCP79412RTC** library:

- **rtcSet1:** Set the RTC date and time using a hard-coded value in the sketch.
- **rtcSet2:** Similar to **rtcSet1**, a different way to hard-code the date and time.
- **rtcSet3:** Set the RTC to the sketch compile date and time.
- **rtc_wire1:** Raspberry Pi Pico example using `Wire1`.
- **SetSerial:** Set the RTC's date, time, and calibration register from the Arduino serial monitor.
- **TimeRTC:** Similar to the example of the same name provided with the **Time** library.
- **PowerOutageLogger:** A comprehensive example that implements a power failure logger using the MCP79412's ability to capture power down and power up times.  Power failure events are logged to the MCP79412's SRAM.  Output is to the Arduino serial monitor.
- **tiny79412_KnockBang:** Demonstrates interfacing an ATtiny45/85 to the MCP79412.

## Enumerations
### ALARM_TYPES_t
##### Description
Symbolic names used with the `enableAlarm()` function described below.
##### Values
- ALM_MATCH_SECONDS -- Triggers alarm when the seconds in the RTC time keeping register match the seconds in the alarm register.
- ALM_MATCH_MINUTES -- Triggers when RTC time minutes match alarm minutes.
- ALM_MATCH_HOURS -- Triggers when RTC time hours match alarm hours.
- ALM_MATCH_DAY -- Triggers when day of the week matches. (Triggers at midnight.)
- ALM_MATCH_DATE -- Triggers when date matches. (Triggers at midnight.)
- ALM_MATCH_DATETIME -- Triggers when seconds, minutes, hours, day of week, date and month all match.
- ALM_DISABLE -- Disables the alarm.

### ALARM_NBR_t
##### Description
Symbolic names used with alarm functions.
##### Values
- ALARM_0
- ALARM_1

### SQWAVE_FREQS_t
##### Description
Symbolic names used with the squareWave() function (described below).
##### Values
- SQWAVE_1_HZ,
- SQWAVE_4096_HZ,
- SQWAVE_8192_HZ,
- SQWAVE_32768_HZ,
- SQWAVE_NONE

## Constructor
### MCP79412RTC(TwoWire& wire)
##### Description
Instantiates an `MCP79412RTC` object.
##### Syntax
`MCP79412RTC myRTC(wire);`
##### Parameters
**wire:** An optional parameter to specify which I2C bus to use. If omitted, defaults to `Wire`. *(TwoWire&)*
##### Returns
None.
##### Example
```c++
MCP79412RTC myRTC;          // to use Wire
// or
MCP79412RTC myRTC(Wire1);   // to use Wire1

```

## Initialization function
### begin()
##### Description
Initializes the I2C bus. Calls `Wire.begin()`.
##### Syntax
`begin();`
##### Parameters
None.
##### Returns
None.
##### Example
```c++
MCP79412RTC myRTC;
void setup() {
    myRTC.begin();
}
```

## Functions for setting and reading the time
### get()
##### Description
Reads the current date and time from the RTC and returns it as a *time_t* value. Returns zero if an I2C error occurs (RTC not present, etc.).
##### Syntax
`RTC.get();`
##### Parameters
None.
##### Returns
Current date and time *(time_t)*
##### Example
```c++
time_t myTime;
myTime = RTC.get();
```

### set(time_t t)
##### Description
Sets the RTC date and time to the given *time_t* value.
##### Syntax
`RTC.set(t);`
##### Parameters
**t:** The date and time to set the RTC to *(time_t)*
##### Returns
None.
##### Example
```c++
//this example first sets the system time (maintained by the Time library) to
//a hard-coded date and time, and then sets the RTC from the system time.
//the setTime() function is part of the Time library.
setTime(23, 31, 30, 13, 2, 2009);   //set the system time to 23h31m30s on 13Feb2009
RTC.set(now());                     //set the RTC from the system time
```

### read(tmElements_t &tm)
##### Description
Reads the current date and time from the RTC and returns it as a *tmElements_t* structure. Returns *false* if an I2C error occurs (RTC not present, etc.).  See the [Arduino Time library](https://www.arduino.cc/playground/Code/Time) for details on the *tmElements_t* structure.
##### Syntax
`RTC.read(tm);`
##### Parameters
**tm:** Address of a *tmElements_t* structure to which the date and time are returned.
##### Returns
False if an I2C error occurred, else true.  The date and time read from the RTC are returned to the **tm** parameter.
##### Example
```c++
tmElements_t tm;
RTC.read(tm);
Serial.print(tm.Hour, DEC);
Serial.print(':');
Serial.print(tm.Minute,DEC);
Serial.print(':');
Serial.println(tm.Second,DEC);
```

### write(tmElements_t &tm)
##### Description
Sets the RTC to the date and time given by a *tmElements_t* structure.
##### Syntax
`RTC.write(tm);`
##### Parameters
**tm:** Address of a *tmElements_t* structure used to set the date and time.
##### Returns
None.
##### Example
```c++
tmElements_t tm;
tm.Hour = 23;             //set the tm structure to 23h31m30s on 13Feb2009
tm.Minute = 31;
tm.Minute = 30;
tm.Day = 13;
tm.Month = 2;
tm.Year = 2009 - 1970;    //tmElements_t.Year is the offset from 1970
RTC.write(tm);            //set the RTC from the tm structure
```

### isRunning()
##### Description
Returns a boolean value indicating whether the RTC's oscillator is running.  When there is no backup battery present, the RTC will reset when it is next powered up, and the oscillator will not be running.  Setting the time with `RTC.set()` or `RTC.write()` starts the oscillator.
##### Syntax
`RTC.isRunning();`
##### Parameters
None.
##### Returns
True if the RTC's oscillator is running, else false *(boolean)*
##### Example
```c++
if ( RTC.isRunning() )
	//do something
else
	//do something else
```

## Alarm functions
The MCP79412 RTC has two alarms (Alarm-0 and Alarm-1) that can be used separately or simultaneously.  When an alarm is triggered, a flag is set in the RTC that can be detected with the `alarm()` function below.  Optionally, the RTC's Multi-Function Pin (MFP) can be driven to either a low or high logic level when an alarm is triggered.  When using the MFP with both alarms, be sure to read the comments on the `alarmPolarity()` function below.

### setAlarm(byte alarmNumber, time_t alarmTime)
##### Description
Sets an alarm date and time.  This *sets* the alarm registers only, it does not *enable* the alarm, this is done using the `enableAlarm()` function.  *alarmNumber* is 0 or 1, but is ruthlessly masked to ensure a valid value.  Note that depending on the alarm type chosen (see `enableAlarm()` below), only selected date or time parts may act as alarm critera.  Nevertheless, valid values should be specified in the *alarmTime* parameter.
##### Syntax
`RTC.setAlarm(alarmNumber, alarmTime);`
##### Parameters
**alarmNumber:** ALARM_0 or ALARM_1 *(byte)*  
**alarmTime:** Date and time to set the alarm to *(time_t)*  
##### Returns
None.
##### Example
```c++
//set alarm-1 for 30 seconds after midnight on 21Dec2012
tmElements_t tm;
tm.Hour = 0;
tm.Minute = 0;
tm.Second = 30;
tm.Year = CalendarYrToTm(2012);
tm.Month = 12;
tm.Day = 21;
RTC.setAlarm(ALARM_1, makeTime(tm));
```

### enableAlarm(byte alarmNumber, byte alarmType)
##### Description
Enable or disable the given alarm.
##### Syntax
`RTC.enableAlarm(alarmNumber, alarmType);`
##### Parameters
**alarmNumber:** ALARM_0 or ALARM_1 *(byte)*  
**alarmType:** One of the following: ALM_MATCH_SECONDS, ALM_MATCH_MINUTES, ALM_MATCH_HOURS, ALM_MATCH_DAY, ALM_MATCH_DATE, ALM_MATCH_DATETIME, ALM_DISABLE.  (ALM_MATCH_DATETIME triggers the alarm when seconds, minutes, hours, day, date and month *all* match.)
##### Returns
None.
##### Example
```c++
//disable alarm-0
RTC.enableAlarm(ALARM_0, ALM_DISABLE);

//enable alarm-1 to trigger when the minutes match.
//assuming alarm-1 is set as in the example above, this will trigger the
//alarm every hour, on the hour (minutes=0).
RTC.enableAlarm(ALARM_1, ALM_MATCH_MINUTES);

//enable alarm-1 to trigger when the seconds match.
//assuming alarm-1 is set as in the example above, this will trigger the
//alarm once a minute, at 30 seconds past the minute.
RTC.enableAlarm(ALARM_1, ALM_MATCH_SECONDS);
```

### alarm(byte alarmNumber)
##### Description
Tests whether the given alarm has been triggered, and returns a corresponding boolean value.  Clears the alarm flag to ensure that the next trigger event can be trapped.
##### Syntax
`RTC.alarm(byte alarmNumber);`
##### Parameters
**alarmNumber:** ALARM_0 or ALARM_1 *(byte)*
##### Returns
True if the alarm was triggered, else false *(boolean)*
##### Example
```c++
if ( RTC.alarm(ALARM_0) )
    //alarm-0 has triggered
else
    //alarm-0 has not triggered
```

### alarmPolarity(boolean polarity)
##### Description
Specifies the logic level on the Multi-Function Pin (MFP) when an alarm is triggered.  The default is LOW.  When both alarms are active, the two are ORed together to determine the level of the MFP.  With alarm polarity set to LOW (the default), this causes the MFP to go low only when BOTH alarms are triggered.  With alarm polarity set to HIGH, the MFP will go high when EITHER alarm is triggered.  Note that the state of the MFP is independent of the RTC's (so-called) alarm "interrupt" flags, and that the `alarm()` function will indicate when an alarm is triggered regardless of the polarity.
##### Syntax
`RTC.alarmPolarity(boolean polarity);`
##### Parameters
**polarity:** HIGH or LOW *(boolean)*
##### Returns
None.
##### Example
```c++
RTC.alarmPolarity(HIGH);    //drives MFP high when an alarm is triggered
```

## Functions for reading and writing static RAM (SRAM)
The MCP79412 RTC has 64 bytes of battery-backed SRAM that can be read and written with the following functions using addresses between 0 and 63.  Addresses passed to these functions are constrained to the valid range by an AND function.

### sramWrite(byte addr, byte value)
##### Description
Writes a single byte to the SRAM.
##### Syntax
`RTC.sramWrite(addr, value);`
##### Parameters
**addr:** SRAM address to write *(byte)*  
**value:** Value to write *(byte)*  
##### Returns
None.
##### Example
```c++
RTC.sramWrite(3, 14);   //write the value 14 to SRAM address 3
```

### sramWrite(byte addr, byte *values, byte nBytes)
##### Description
Writes multiple bytes to consecutive SRAM locations.  *nBytes* must be between 1 and 31.  Invalid values of *nBytes*, or combinations of *addr* and *nBytes* that would result in addressing past the last byte of SRAM will result in no action.
##### Syntax
`RTC.sramWrite(addr, values, nBytes);`
##### Parameters
**addr:** First SRAM address to write *(byte)*  
**value:** An array of values to write _(*byte)_  
**nBytes:** Number of bytes to write *(byte)*  
##### Returns
None.
##### Example
```c++
//write 1, 2, ..., 8 to the first eight SRAM locations
byte buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
RTC.sramWrite(0, buf, 8);
```

### sramRead(byte addr)
##### Description
Reads a single byte from SRAM.
##### Syntax
`RTC.sramRead(addr);`
##### Parameters
**addr:** SRAM address to read *(byte)*
##### Returns
The value read *(byte)*
##### Example
```c++
byte val;
val = RTC.sramRead(3);  //read the value from SRAM location 3
```

### sramRead(byte addr, byte *values, byte nBytes)
##### Description
Reads multiple bytes from consecutive SRAM locations.  nBytes must be between 1 and 32.  Invalid values of *nBytes*, or combinations of *addr* and *nBytes* that would result in addressing past the last byte of SRAM will result in no action.
##### Syntax
`RTC.sramRead(addr, values, nBytes);`
##### Parameters
**addr:** First SRAM address to read *(byte)*  
**values:** An array to receive the read values _(*byte)_  
**nBytes:** Number of bytes to read *(byte)*  
##### Returns
No function value returned.  Bytes read from SRAM are returned to the **values** array.
##### Example
```c++
//read the last eight locations of SRAM into buf
byte buf[8];
RTC.sramRead(56, buf, 8);
```

## Functions for Reading and writing EEPROM
The MCP79412 RTC has 128 bytes of non-volatile EEPROM that can be read and written with the following functions using addresses between 0 and 127.  Addresses passed to these functions are constrained to the valid range by an AND function.

EEPROM is paged memory with a page size of 8 bytes; when writing multiple bytes, this this limits the number of bytes that can be written at one time to 8.  Page writes must start on a page boundary.

### eepromWrite(byte addr, byte value)
##### Description
Writes a single byte to EEPROM.
##### Syntax
`RTC.eepromWrite(addr, value);`
##### Parameters
**addr:** EEPROM address to write *(byte)*  
**value:** Value to write *(byte)*  
##### Returns
None.
##### Example
```c++
RTC.eepromWrite(42, 55);   //write the value 55 to EEPROM address 42
```

### eepromWrite(byte addr, byte *values, byte nBytes)
##### Description
Writes a page (8 bytes) or less to EEPROM.  *addr* should be a page start address (0, 8, ..., 120), but if not, is ruthlessly coerced into a valid value with an AND function.  *nBytes* must be between 1 and 8, other values result in no action.
##### Syntax
`RTC.eepromWrite(addr, values, nBytes);`
##### Parameters
**addr:** First EEPROM address to write *(byte)*  
**value:** An array of values to write _(*byte)_  
**nBytes:** Number of bytes to write *(byte)*  
##### Returns
None.
##### Example
```c++
//write 1, 2, ..., 8 to the first eight EEPROM locations
byte buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
RTC.eepromWrite(0, buf, 8);
```

### eepromRead(byte addr)
##### Description
Reads a single byte from EEPROM and returns the value.
##### Syntax
`RTC.eepromRead(byte addr);`
##### Parameters
**addr:** EEPROM address to read *(byte)*
##### Returns
The value read *(byte)*
##### Example
```c++
byte val;
val = RTC.eepromRead(42);  //read the value from EEPROM location 42
```

### eepromRead(byte addr, byte *values, byte nBytes)
##### Description
Reads multiple bytes from consecutive EEPROM locations.  *nBytes* must be between 1 and 32. Invalid values of *nBytes*, or combinations of *addr* and *nBytes* that would result in addressing past the last byte of EEPROM will result in no action.
##### Syntax
`RTC.eepromRead(addr, values, nBytes);`
##### Parameters
**addr:** First EEPROM address to read *(byte)*  
**values:** An array to receive the read values _(*byte)_  
**nBytes:** Number of bytes to read *(byte)*  
##### Returns
No function value returned.  The bytes read from EEPROM are returned to the **values** array.
##### Example
```c++
//read the last eight locations of EEPROM into buf
byte buf[8];
RTC.eepromRead(120, buf, 8);
```

## Calibration, power failure, and other functions
### calibWrite(int value)
##### Description
Writes the given value to the RTC calibration register.  This is an adjustment factor in PPM (approximately), and must be between -127 and 127.  Negative numbers cause the RTC to run faster, positive numbers cause it to run slower.
##### Syntax
`RTC.calibWrite(int value);`
##### Parameters
**value:** The calibration value to set, between -127 and 127 *(int)*
##### Returns
None.
##### Example
```c++
RTC.calibWrite(13);     //makes the RTC run slower by 13 parts per million.
RTC.calibWrite(-42);    //makes the RTC run faster by 42 parts per million.
```

### calibRead()
##### Description
Reads the RTC calibration register.
##### Syntax
`RTC.calibRead();`
##### Parameters
None.
##### Returns
RTC calibration register value *(int)*
##### Example
```c++
int value;
value = RTC.calibRead();
```

### powerFail(time_t *powerDown, time_t *powerUp)
##### Description
Returns a boolean value (true or false) to indicate whether a power failure has occurred. If a power failure occurred, the power down and power up timestamps are returned in the variables given by the caller, the RTC's power fail flag is reset and the power up/down timestamps are cleared.

Note that the power down and power up timestamp registers do not contain values for seconds or for the year. The returned time stamps will therefore contain the current year from the RTC. However, there is the possibility that a power outage spans from one year to the next. If this occurs, the power down timestamp would appear to be at a later time than the power up timestamp; if this is encountered, `powerFail()` will subtract one year from the power down timestamp before returning it.

Still, there is an assumption that the timestamps are being read in the same year as that when the power up occurred.  If this is not the case the year in the returned timestamp will be invalid.

Finally, note that once the RTC records a power outage, it must be cleared before another can be recorded. If two power outages occur before `powerFail()` is called again, the time stamps for the earlier outage will be returned and the timestamps for the second outage will be lost.
##### Syntax
`RTC.powerFail(powerDown, powerUp);`
##### Parameters
**powerDown:** Pointer to a *time_t* variable to hold the returned power down timestamp.  
**powerUp:** Pointer to a *time_t* variable to hold the returned power up timestamp.  
##### Returns
True if a power failure occurred, else false *(boolean)*
##### Example
```c++
time_t powerDown, powerUp;    //power outage timestamps
if ( RTC.powerFail(&powerDown, &powerUp) ) {
    //do something
else
    //do something else
```

### squareWave(byte freq)
##### Description
Enables or disables the square wave output on the multi-function pin (MFP).
##### Syntax
`RTC.squareWave(byte freq);`
##### Parameters
**freq:** One of the following: SQWAVE_1_HZ, SQWAVE_4096_HZ, SQWAVE_8192_HZ, SQWAVE_32768_HZ, SQWAVE_NONE *(byte)*
##### Returns
None.
##### Example
```c++
RTC.squareWave(SQWAVE_1_HZ);    //output a 1Hz square wave on the MFP
```
--------------------------------------------------------------------------------

### out(boolean level)
##### Description
Sets the logic level on the MFP when it's not being used as a square wave or alarm output.  The default value after the RTC chip is reset is HIGH.
##### Syntax
`RTC.out(boolean level);`
##### Parameters
**level**: HIGH or LOW *(boolean)*
##### Returns
None.
##### Example
```c++
RTC.out(LOW);   //set the MFP to a low logic level
```
--------------------------------------------------------------------------------

### idRead(byte *uniqueID)
##### Description
Reads the 64-bit unique ID from the RTC.
##### Syntax
`RTC.idRead(byte *uniqueID);`
##### Parameters
**uniqueID:** An 8-byte array to receive the unique ID _(*byte)_
##### Returns
No function value returned.  The RTC's ID is returned to the **uniqueID** array.
##### Example
```c++
byte buf[8];
RTC.idRead(buf);
```
--------------------------------------------------------------------------------

### vbaten(boolean enable)
##### Description
Set or clear the VBATEN bit. Setting the bit powers the clock and SRAM from the backup battery when Vcc falls. Note that setting the time via set() or write() sets the VBATEN bit.                 
##### Syntax
`RTC.vbaten(boolean enable);`
##### Parameters
**enable:** true or false *(boolean)*
##### Returns
None.
##### Example
```c++
RTC.vbaten(false);
```
--------------------------------------------------------------------------------

### getEUI64(byte *uniqueID)
##### Description
Returns an EUI-64 ID. For an MCP79412, calling this function is equivalent to calling `idRead()`. For an MCP79411, the EUI-48 ID is converted to EUI-64. Caller must provide an 8-byte array to contain the results.
##### Syntax
`RTC.getEUI64(byte *uniqueID);`
##### Parameters
**uniqueID:** An 8-byte array to receive the EUI-64 unique ID _(*byte)_
##### Returns
No function value returned.  The EUI-64 ID is returned to the **uniqueID** array.
##### Example
```c++
byte buf[8];
RTC.getEUI64(buf);
```
