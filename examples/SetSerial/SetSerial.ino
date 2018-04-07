// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch. Displays the date and time from an MCP79412 RTC
// every second.
//
// Set the date and time by entering the following on the Arduino
// serial monitor:
//    Syear,month,day,hour,minute,second,
//
// Where
//    year can be two or four digits,
//    month is 1-12,
//    day is 1-31,
//    hour is 0-23, and
//    minute and second are 0-59.
//
// Set the calibration register by entering the following:
//    Cnnn,
//
// Where nnn can be a positive or negative value, e.g. "c4" or "c-42".
//
// Entering the final comma delimiter (after "second") will avoid a
// one-second timeout and will allow the RTC to be set more accurately.
//
// No validity checking is done, invalid values or incomplete syntax
// in the input will result in an incorrect RTC setting.
//
// Jack Christensen 28Aug2013

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <Streaming.h>      // http://arduiniana.org/libraries/streaming/
#include <TimeLib.h>        // https://github.com/PaulStoffregen/Time

void setup()
{
    byte rtcID[8];

    Serial.begin(115200);

    // setSyncProvider() causes the Time library to synchronize with the
    // external RTC by calling RTC.get() every five minutes by default.
    setSyncProvider(RTC.get);
    Serial << endl << F("RTC Sync");
    if (timeStatus() != timeSet) Serial << F(" FAIL!");
    Serial << endl;

    RTC.idRead(rtcID);
    Serial << F("RTC ID = ");
    for (int i=0; i<8; ++i) {
        if (rtcID[i] < 16) Serial << '0';
        Serial << _HEX(rtcID[i]);
    }
    Serial << endl;

    RTC.getEUI64(rtcID);
    Serial << F("EUI-64 = ");
    for (int i=0; i<8; ++i) {
        if (rtcID[i] < 16) Serial << '0';
        Serial << _HEX(rtcID[i]);
    }
    Serial << endl;

    Serial << F("Calibration Register = ") << RTC.calibRead() << endl;
}

void loop()
{
    static time_t tLast;
    time_t t;
    tmElements_t tm;
    int cmdChar, y, oldCal, newCal;

    // check for input, first character is a command, "S" to set date/time, or "C" to set the calibration register
    if (Serial.available()) {
        cmdChar = Serial.read();

        switch (cmdChar) {
            case 'S':
            case 's':
                // note that the tmElements_t Year member is an offset from 1970,
                // but the RTC wants the last two digits of the calendar year.
                // use the convenience macros from TimeLib.h to do the conversions.
                y = Serial.parseInt();
                if (y >= 100 && y < 1000)
                    Serial << F("Error: Year must be two digits or four digits!") << endl;
                else {
                    if (y >= 1000)
                        tm.Year = CalendarYrToTm(y);
                    else    //(y < 100)
                        tm.Year = y2kYearToTm(y);
                    tm.Month = Serial.parseInt();
                    tm.Day = Serial.parseInt();
                    tm.Hour = Serial.parseInt();
                    tm.Minute = Serial.parseInt();
                    tm.Second = Serial.parseInt();
                    t = makeTime(tm);
                    RTC.set(t);        // use the time_t value to ensure correct weekday is set
                    setTime(t);
                    Serial << F("RTC set to: ");
                    printDateTime(t);
                    Serial << endl;
                }
                break;

            case 'C':
            case 'c':
                newCal = Serial.parseInt();
                oldCal = RTC.calibRead();
                RTC.calibWrite(newCal);
                Serial << F("Calibration changed from ") << oldCal << F(" to ") << RTC.calibRead() << endl;
                break;

            default:
                Serial << endl << F("Unrecognized command: ") << (char)cmdChar << endl;
                break;
        }

        // dump any extraneous input
        while (Serial.available() > 0) Serial.read();
    }

    t = now();
    if (t != tLast) {
        tLast = t;
        printDateTime(t);
        Serial << endl;
    }
}

// print date and time to Serial
void printDateTime(time_t t)
{
    printDate(t);
    Serial << ' ';
    printTime(t);
}

// print time to Serial
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

// print date to Serial
void printDate(time_t t)
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

// Print an integer in "00" format (with leading zero),
// followed by a delimiter character to Serial.
// Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}
