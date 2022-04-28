// Arduino MCP79412RTC Library
// https://github.com/JChristensen/MCP79412RTC
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Example sketch: Power Outage Logger using Microchip MCP79412 RTC.
// Assumes the RTC is running and set to UTC.
// A maximum of 7 outages (power down/up times) can be logged in the
// RTC's SRAM.
// The logging data structure is automatically initialized if not
// present. An initialization can be forced with a button switch
// connected from pin 9 to ground. Hold the button down
// while resetting the MCU to initialize the logging data.
//
// Jack Christensen 23Aug2012

#include <MCP79412RTC.h>    // https://github.com/JChristensen/MCP79412RTC
#include <Streaming.h>      // https://github.com/janelia-arduino/Streaming
#include <Timezone.h>       // https://github.com/JChristensen/Timezone

MCP79412RTC myRTC;

void setup()
{
    constexpr uint8_t initButton {9};
    pinMode(initButton, INPUT_PULLUP);
    myRTC.begin();
    Serial.begin(115200);
    Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );

    setSyncProvider(myRTC.get);     // the function to get the time from the RTC
    Serial << "RTC SYNC";
    if (timeStatus()!= timeSet) Serial << " FAIL";
    Serial << endl;

    if (!digitalRead(initButton)) logClear();
    logOutage();
    //myRTC.dumpSRAM();
}

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240};  // Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};   // Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);
TimeChangeRule *tcr;            // pointer to the time change rule, used to get TZ abbrev

void loop()
{
    // nothing here in loop() has anything to do with logging power outages,
    // we just print the time once a minute so that something is happening.
    static time_t lastUTC;
    time_t utc = now();
    if (minute(utc) != minute(lastUTC)) {
        lastUTC = utc;
        time_t local = myTZ.toLocal(utc, &tcr);
        Serial << endl;
        printTime(utc, "UTC");
        printTime(local, tcr -> abbrev);
    }
}

// constants for logging
constexpr uint8_t
    firstOutageAddr {0x08},     // address of first outage timestamps in RTC SRAM
    outageLength    {8},        // 8 data bytes for each outage (start and end timestamps, both are time_t values)
    maxOutages      {7},        // maximum number of outage timestamp pairs that can be stored in SRAM
    lastOutageAddr  {firstOutageAddr + outageLength * (maxOutages - 1)},    // last outage address
    appID           {1},        // appID and 4 bytes of the RTC ID are stored in sram to provide
                                // a way to recognize that the logging data structure has been initialized
    rtcID           {0x00},     // lower 4 bytes of RTC unique ID are stored at sram addr 0x00
    appAddr         {0x04},     // address of appID in sram
    nbrOutagesAddr  {0x05},     // address containing number of outages currently stored in SRAM
    nextOutageAddr  {0x06},     // address containing pointer to next outage
    rfuAddr         {0x07};     // reserved for future use

// initialize the log data structure in the RTC SRAM if needed.
// log a new outage if one occurred.
// print out the outages logged.
void logOutage() {
    union {
        uint8_t b[8];
        struct {
            uint32_t hi;
            uint32_t lo;
        };
    } uniqueID;                 // 8-byte RTC "unique ID" with access to upper and lower halves

    myRTC.idRead(uniqueID.b);           // get the RTC's ID
    uint32_t loID = read32(rtcID);      // if already initialized, the lower half of the ID is stored in SRAM,
    uint8_t app = myRTC.sramRead(appAddr);  // and also the app ID
    Serial << "RTC ID";
    for (uint8_t i=0; i<8; i++) {
        Serial << (uniqueID.b[i] < 16 ? " 0" : " ") << _HEX(uniqueID.b[i]);
    }
    Serial << endl;

    if ( loID != uniqueID.lo || app != appID ) {            // logging initialized?
        write32(rtcID, uniqueID.lo);                        // least significant half of the RTC unique ID
        myRTC.sramWrite(appAddr, appID);                    // app ID
        myRTC.sramWrite(nbrOutagesAddr, 0);                 // number of outages
        myRTC.sramWrite(nextOutageAddr, firstOutageAddr);   // next location for outage times
        myRTC.sramWrite(rfuAddr, 0);                        // reserved for future use
        Serial << "Logging initialized" << endl;            // no, do it now
    }

    // if an outage has occurred, record it
    time_t powerDown, powerUp;  // power outage timestamps
    uint8_t nOutage;            // number of outages stored in sram
    uint8_t nextOutage;         // address of next outage timestamps in sram
    if ( myRTC.powerFail(&powerDown, &powerUp) ) {
        nOutage = myRTC.sramRead(nbrOutagesAddr);
        nextOutage = myRTC.sramRead(nextOutageAddr);
        write32(nextOutage, powerDown);
        write32(nextOutage + 4, powerUp);
        nextOutage += outageLength;
        if (nextOutage > lastOutageAddr) nextOutage = firstOutageAddr;
        myRTC.sramWrite(nextOutageAddr, nextOutage);
        if (nOutage < maxOutages) myRTC.sramWrite(nbrOutagesAddr, ++nOutage);
    }

    // print out all the outages logged
    nOutage = myRTC.sramRead(nbrOutagesAddr);
    nextOutage = myRTC.sramRead(nextOutageAddr);
    uint8_t outageAddr = nextOutage - outageLength;
    if (outageAddr < firstOutageAddr) outageAddr = lastOutageAddr;
    Serial << endl << "Power outages logged: " << _DEC(nOutage) << endl;
    for (uint8_t i=nOutage; i>0; i--) {
        powerDown = read32(outageAddr);
        powerUp = read32(outageAddr + 4);
        Serial << endl << _DEC(i) << ": Power down ";
        printTime(myTZ.toLocal(powerDown, &tcr), tcr -> abbrev);
        Serial << _DEC(i) << ": Power up   ";
        printTime(myTZ.toLocal(powerUp, &tcr), tcr -> abbrev);
        outageAddr -= outageLength;
        if (outageAddr < firstOutageAddr) outageAddr = lastOutageAddr;
    }
}

// initialize the logging data structure and log data
void logClear()
{
    for (uint8_t i=0; i<lastOutageAddr + outageLength; i++) {
        myRTC.sramWrite(i, 0);
    }
}

// write a time_t or other uint32_t value to sram starting at addr
void write32(uint8_t addr, uint32_t t)
{
    union {
        uint8_t b[4];
        uint32_t t;
    } i;

    i.t = t;
    myRTC.sramWrite(addr, i.b, 4);
}

// read a time_t or other uint32_t value from sram starting at addr
time_t read32(uint8_t addr)
{
    union {
        uint8_t b[4];
        time_t t;
    } i;

    myRTC.sramRead(addr, i.b, 4);
    return i.t;
}

// Print time with time zone
void printTime(time_t t, const char *tz)
{
    sPrintI00(hour(t));
    sPrintDigits(minute(t));
    sPrintDigits(second(t));
    Serial << ' ' << dayShortStr(weekday(t)) << ' ';
    sPrintI00(day(t));
    Serial << ' ' << monthShortStr(month(t)) << ' ' << year(t) << ' ' << tz << endl;
}

// Print an integer in "00" format (with leading zero).
// Input value assumed to be between 0 and 99.
void sPrintI00(int val)
{
    if (val < 10) Serial << '0';
    Serial << val;
    return;
}

// Print an integer in ":00" format (with leading zero).
// Input value assumed to be between 0 and 99.
void sPrintDigits(int val)
{
    Serial << ':';
    if(val < 10) Serial << '0';
    Serial << val;
}
