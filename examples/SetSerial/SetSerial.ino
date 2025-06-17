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

MCP79412RTC myRTC;

void setup()
{
    Serial.begin(115200);
    Serial << F( "\n" __FILE__ "\n" __DATE__ " " __TIME__ "\n" );
    myRTC.begin();

    // setSyncProvider() causes the Time library to synchronize with the
    // external RTC by calling myRTC.get() every five minutes by default.
    setSyncProvider([](){return myRTC.get();});
    Serial << endl << F("RTC Sync");
    if (timeStatus() != timeSet) Serial << F(" FAIL!");
    Serial << endl;

    uint8_t rtcID[8];
    myRTC.idRead(rtcID);
    Serial << F("RTC ID = ");
    for (int i=0; i<8; ++i) {
        if (rtcID[i] < 16) Serial << '0';
        Serial << _HEX(rtcID[i]);
    }
    Serial << endl;

    myRTC.getEUI64(rtcID);
    Serial << F("EUI-64 = ");
    for (int i=0; i<8; ++i) {
        if (rtcID[i] < 16) Serial << '0';
        Serial << _HEX(rtcID[i]);
    }
    Serial << endl;

    Serial << F("Calibration Register = ") << myRTC.calibRead() << endl;
}

void loop()
{
    // check for input, to set rtc time or calibration
    if (Serial.available()) setRTC();

    // print time and date every second
    static time_t tLast;
    time_t t { now() };
    if (t != tLast) {
        tLast = t;
        printTime(t);
    }
}

void setRTC()
{
    // first character is a command, "S" to set date/time, or "C" to set the calibration register
    int cmdChar = Serial.read();

    switch (cmdChar) {
        case 'S':
        case 's':
            delay(25);  // wait for all the input to arrive
            // check for input to set the RTC, minimum length is 13, i.e. yy,m,d,h,m,s<nl>
            if (Serial.available() < 13) {
                while (Serial.available()) Serial.read();  // dump extraneous input
                Serial << F("Input error or timeout, try again.\n");
            }
            else {
                // note that the tmElements_t Year member is an offset from 1970,
                // but the RTC wants the last two digits of the calendar year.
                // use the convenience macros from TimeLib.h to do the conversions.
                int y = Serial.parseInt();
                if (y >= 100 && y < 1000)
                    Serial << F("Error: Year must be two digits or four digits!\n");
                else {
                    tmElements_t tm;
                    if (y >= 1000)
                        tm.Year = CalendarYrToTm(y);
                    else    //(y < 100)
                        tm.Year = y2kYearToTm(y);
                    tm.Month = Serial.parseInt();
                    tm.Day = Serial.parseInt();
                    tm.Hour = Serial.parseInt();
                    tm.Minute = Serial.parseInt();
                    tm.Second = Serial.parseInt();
                    if (tm.Month == 0 || tm.Day == 0) {
                        while (Serial.available()) Serial.read();  // dump extraneous input
                        Serial << F("Input error or timeout, try again.\n");
                    }
                    else {
                        time_t t = makeTime(tm);
                        myRTC.set(t);        // use the time_t value to ensure correct weekday is set
                        setTime(t);
                        Serial << F("RTC set to: ");
                        printTime(t);
                    }
                }
            }
            break;

        case 'C':
        case 'c':
            delay(25);  // wait for all the input to arrive
            if (Serial.available() < 2) {   // minimum valid input at this point is 2 chars
                while (Serial.available()) Serial.read();  // dump extraneous input
                Serial << F("Input error or timeout, try again.\n");
            }
            else {
                int newCal = Serial.parseInt();
                int oldCal = myRTC.calibRead();
                myRTC.calibWrite(newCal);
                Serial << F("Calibration changed from ") << oldCal << F(" to ") << myRTC.calibRead() << endl;
            }
            break;

        default:
            Serial << endl << F("Unrecognized command: ") << (char)cmdChar << endl;
            break;
    }

    // dump any extraneous input
    while (Serial.available()) Serial.read();
}

// format and print a time_t value
void printTime(const time_t t)
{
    char buf[25];
    char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d",
        hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t));
    Serial.println(buf);
}
