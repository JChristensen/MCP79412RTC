// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch to set the RTC date and time to a hard-coded value,
// using a tmElements_t structure. The same structure is then used to
// read the RTC once per second. This sketch only uses the Time
// library for the tmElements_t definition.
//
// This is just a simple demonstration of setting the RTC time.
// Note that each time the sketch is run (or the microcontroller is
// reset), the RTC will be set to the same hard-coded time, so this
// example may be of limited usefulness as an actual clock.

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <TimeLib.h>        // https://github.com/PaulStoffregen/Time

tmElements_t tm;

void setup()
{
    delay(2000);
    Serial.begin(9600);

    tm.Hour = 23;             // set the tm structure to 23h31m30s on Fri 13Feb2009
    tm.Minute = 31;
    tm.Second = 30;
    tm.Year = 2009 - 1970;    // tmElements_t.Year is the offset from 1970.
    tm.Month = 2;
    tm.Day = 13;
    tm.Wday = dowFriday;      // See enum in Time.h: Sun=1, Mon=2, ... Sat=7
    RTC.write(tm);            // set the RTC from the tm structure
}

void loop()
{
    RTC.read(tm);
    Serial.print(tm.Hour, DEC);
    Serial.print(':');
    Serial.print(tm.Minute,DEC);
    Serial.print(':');
    Serial.print(tm.Second,DEC);
    Serial.print(' ');
    Serial.print(tm.Year + 1970, DEC);
    Serial.print('-');
    Serial.print(tm.Month, DEC);
    Serial.print('-');
    Serial.print(tm.Day, DEC);
    Serial.println();
    delay(1000);
}
