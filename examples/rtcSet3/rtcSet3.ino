// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch to set the RTC date and time to the sketch compile
// time. Re-running this sketch by pressing the reset button, etc.,
// will cause the RTC time to be reset to the *same* compile time.
// Upload the sketch again to recompile it with the current time.

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC

MCP79412RTC myRTC;
time_t compileTime(const uint32_t fudge=10);    // function prototype

void setup()
{
    myRTC.begin();
    delay(2000);
    Serial.begin(115200);

    setTime(compileTime());    // set the system time to the sketch compile time
    myRTC.set(now());          // set the RTC from the system time
}

void loop()
{
    printTime(now());
    delay(1000);
}

// A function to return the compile date and time as a time_t value.
// The fudge argument (seconds) is used to adjust for compile and upload time, YMMV.
time_t compileTime(const uint32_t fudge)
{
    //const uint32_t fudge(10);        // fudge factor to allow for compile time (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[3], *m;
    tmElements_t tm;
    time_t t;

    strncpy(chMon, compDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    t = makeTime(tm);
    return t + fudge;        // add fudge factor to allow for compile time
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
