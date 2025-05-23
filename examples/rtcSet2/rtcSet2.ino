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

MCP79412RTC myRTC;
tmElements_t tm;

void setup()
{
    myRTC.begin();
    delay(2000);
    Serial.begin(115200);

    tm.Hour = 23;             // set the tm structure to 23h31m30s on 27Apr2022
    tm.Minute = 31;
    tm.Second = 30;
    tm.Year = 2022 - 1970;    // tmElements_t.Year is the offset from 1970.
    tm.Month = 4;
    tm.Day = 27;
    tm.Wday = dowFriday;      // See enum in Time.h: Sun=1, Mon=2, ... Sat=7
    myRTC.write(tm);          // set the RTC from the tm structure
}

void loop()
{
    myRTC.read(tm);
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
