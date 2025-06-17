// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// rtc_wire1.ino
// Example sketch with the RTC on the Wire1 bus.
// Tested with Raspberry Pi Pico using the Arduino-Pico core,
// https://github.com/earlephilhower/arduino-pico

#include <MCP79412RTC.h>    // http://github.com/JChristensen/MCP79412RTC

constexpr int sdaPin {26}, sclPin {27}; // I2C pins for Wire1 (your pins may vary)
MCP79412RTC myRTC(Wire1);

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000) delay(50);

    Wire1.setSDA(sdaPin); Wire1.setSCL(sclPin);
    myRTC.begin();
    setSyncProvider([](){return myRTC.get();}); // the function to get the time from the RTC
    if (timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
}

void loop()
{
    Serial.printf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d\n", year(), month(), day(),
        hour(), minute(), second());
    delay(1000);
}
