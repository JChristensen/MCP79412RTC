// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch to set the RTC date and time to a hard-coded value.
// This is just a simple demonstration of setting the RTC time.
// Note that each time the sketch is run (or the microcontroller is
// reset), the RTC will be set to the same hard-coded time, so this
// example may be of limited usefulness as an actual clock.

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <TimeLib.h>        // https://github.com/PaulStoffregen/Time

void setup()
{
    delay(2000);
    Serial.begin(9600);

    setTime(23, 31, 30, 13, 2, 2009);   // set the system time to 23h31m30s on 13Feb2009
                                        // the setTime() function is from the Time.h library.
                                        // setTime(hour, minute, second, day, month, year);
    RTC.set(now());                     // set the RTC from the system time
}

void loop()
{
    printTime(now());
    delay(1000);
}

// Print time (and date) given a time_t value
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
    Serial.print(dayShortStr(weekday(t)));
    Serial.print(' ');
    printI00(day(t), ' ');
    Serial.print(monthShortStr(month(t)));
    Serial.print(' ');
    Serial.println(year(t));
}

// Print an integer in "00" format (with leading zero),
// followed by a delimiter.
// Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val < 10) Serial.print('0');
    Serial.print(val);
    Serial.print(delim);
    return;
}
