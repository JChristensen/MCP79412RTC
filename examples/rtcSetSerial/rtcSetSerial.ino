// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch to set the RTC by entering a "Set" command on the
// serial monitor. Use a 24-hour clock and enter the command
// (case sensitive) exactly as follows: Set yyyy-mm-dd hh:mm:ss

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <TimeLib.h>        // https://github.com/PaulStoffregen/Time

const uint32_t PRINT_INTERVAL(10000);   // ms between printing the time
uint32_t ms, msLast;

void setup()
{
    delay(2000);
    Serial.begin(9600);

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    Serial.print("RTC SYNC");
    if (timeStatus() !=  timeSet) Serial.print(" FAIL");
    Serial.println();
}

void loop()
{
    ms = millis();
    readCommand();
    if (ms - msLast >= PRINT_INTERVAL) {
        msLast = ms;
        printTime(now());
    }
}

// Read command from the Arduino serial monitor to set the RTC.
// Case-sensitive and must be entered exactly as (24-hour clock):
//   Set yyyy-mm-dd hh:mm:ss
void readCommand()
{
    char cmd[24] = "Set yyyy-mm-dd hh:mm:ss";

    // serial terminal will send 23 char command plus line terminator (0x0A)
    if (Serial.available() >= 24) {             // enough characters for the whole command?
        unsigned int i = 0;                     // yes, read the available characters
        while (Serial.available() > 0) {
            if (i >= sizeof(cmd)) {             // more than we can enjoy
                flushInput();                   // clear out the input buffer
                cmd[sizeof(cmd) - 1] = 0;       // string terminator
                Serial.print("Too long: ");
                Serial.println(cmd);
                return;
            }
            delay(2);                           // let the next character trickle in
            char c = Serial.read();
            if (c >= ' ') cmd[i++] = c;         // printable characters and spaces only
        }
        cmd[i] = 0;                             // put in string terminator

        tmElements_t tmSet;
        if (strncmp(cmd, "Set ", 4) == 0) {
            tmSet.Year = 1000 * (cmd[4] - '0') + 100 * (cmd[5] - '0') + 10 * (cmd[6] - '0') + cmd[7] - '0' - 1970;
            tmSet.Month = 10 * (cmd[9] - '0') + cmd[10] - '0';
            tmSet.Day = 10 * (cmd[12] - '0') + cmd[13] - '0';
            tmSet.Hour = 10 * (cmd[15] - '0') + cmd[16] - '0';
            tmSet.Minute = 10 * (cmd[18] - '0') + cmd[19] - '0';
            tmSet.Second = 10 * (cmd[21] - '0') + cmd[22] - '0';
            time_t tSet = makeTime(tmSet);  // convert to time_t
            setTime(tSet);                  // set the system time
            RTC.set(now());                 // set the rtc
            Serial.println("RTC set!");
            printTime(RTC.get());
            flushInput();                   // discard any extraneous trailing characters
        }
        else {
            Serial.print("Unknown: ");
            Serial.println(cmd);
        }
    }
}

void flushInput()
{
    do {
        delay(2);
        Serial.read();
    } while (Serial.available() > 0);
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
