// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// TimeRTC.ino
// Example sketch showing basic usage.

#include <MCP79412RTC.h>    // http://github.com/JChristensen/MCP79412RTC

MCP79412RTC myRTC;

void setup()
{
    myRTC.begin();
    Serial.begin(115200);
    setSyncProvider(myRTC.get); // the function to get the time from the RTC
    if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
}

void loop()
{
    digitalClockDisplay();
    delay(1000);
}

// digital clock display of the time
void digitalClockDisplay()
{
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(" ");
    Serial.print(month());
    Serial.print(" ");
    Serial.print(year());
    Serial.println();
}

// utility function for digital clock display: prints preceding colon and leading 0
void printDigits(int digits)
{
    Serial.print(":");
    if (digits < 10)
        Serial.print('0');
    Serial.print(digits);
}
