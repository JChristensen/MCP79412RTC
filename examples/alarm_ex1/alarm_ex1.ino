// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// MCP79412RTC Alarm Example Sketch #1
//
// Set Alarm 0 to occur once a minute at 5 seconds after the minute.
// Detect the alarm by polling the MFP output as well as the RTC alarm flag.
//
// Hardware:
// Arduino Uno, MCP79410/11/12 RTC.
// Connect RTC SDA to Arduino pin A4.
// Connect RTC SCL to Arduino pin A5.
//
// Jack Christensen 27Apr2022

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <Streaming.h>      // https://github.com/janelia-arduino/Streaming

MCP79412RTC myRTC;
constexpr uint8_t mfp {3};  // connect to RTC multi-function pin.

void setup()
{
    Serial.begin(115200);
    Serial << F( "\n" __FILE__ "\nCompiled " __DATE__ " " __TIME__ "\n" );
    pinMode(mfp, INPUT_PULLUP);

    // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
    myRTC.begin();
    myRTC.enableAlarm(MCP79412RTC::ALARM_0, MCP79412RTC::ALM_DISABLE);
    myRTC.enableAlarm(MCP79412RTC::ALARM_1, MCP79412RTC::ALM_DISABLE);
    myRTC.alarm(MCP79412RTC::ALARM_0);
    myRTC.alarm(MCP79412RTC::ALARM_1);
    myRTC.squareWave(MCP79412RTC::SQWAVE_NONE);

    // set Alarm 0 to occur at 5 seconds after every minute
    myRTC.setAlarm(MCP79412RTC::ALARM_0, 2022, 1, 1, 0, 0, 5);
    myRTC.enableAlarm(MCP79412RTC::ALARM_0, MCP79412RTC::ALM_MATCH_SECONDS);

    Serial << endl << millis() << " Start ";
    printTime(myRTC.get());
    myRTC.dumpRegs();
}

void loop()
{
    if (!digitalRead(mfp)) {                    // is the mfp pulled low
        Serial << millis() << " MFP LOW ";
        printTime(myRTC.get());
    }
    if ( myRTC.alarm(MCP79412RTC::ALARM_0) ) {  // check alarm flag, clear it if set
        Serial << millis() << " ALARM_0 ";
        printTime(myRTC.get());
    }
    if ( myRTC.alarm(MCP79412RTC::ALARM_1) ) {  // check alarm flag, clear it if set
        Serial << millis() << " ALARM_1 ";
        printTime(myRTC.get());
    }

    delay(100);     // no need to bombard the RTC continuously
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
