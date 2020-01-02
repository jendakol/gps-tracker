#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define GT_PIN_GPS_RX 4
#define GT_PIN_GPS_TX 3

#define GT_PIN_LED_RED 8
#define GT_PIN_LED_GREEN 7
#define GT_PIN_LED_BLUE 6
#define GT_PIN_BUZZER A3

#define BUZZER_ENABLED true

#define GPX_LOC_ACCURACY 8
#define GPX_INTERVAL 2000
#define GPX_FOOTER F("</trkseg>\r\n</trk>\r\n</gpx>")
#define GPX_FOOTER_LENGTH  27

#define FILE_OPEN_MODE O_READ | O_WRITE | O_CREAT

#define TIMEZONE 1

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GT_PIN_GPS_RX, GT_PIN_GPS_TX);

File dataFile;
char filename[25];

TinyGPSLocation last_location;
TinyGPSDate last_date;
TinyGPSTime last_time;
TinyGPSHDOP last_hdop;
TinyGPSAltitude last_alti;
TinyGPSInteger last_sats;

static void beep(int length);

static void beepAndBlinkRed(int length);

static void beepAndBlinkGreen(int length);

static void smartDelay(unsigned long ms);

void writeToGpx();

void resetLEDs();

bool gpsValid();

void snapshotGPS();

void cardNotReady();

void initGPS() {
    gpsSerial.begin(9600);

    gpsSerial.print(F("$PUBX,41,1,0007,0003,4800,0*13\r\n"));
    gpsSerial.flush();
    delay(50);
    if (gpsSerial.available() > 0) {
        gpsSerial.begin(4800);
        gpsSerial.flush();
        delay(50);
    }

    // TODO power saving mode?
}

unsigned long lastPrevention = 0;

void preventOverflow() {
    if (millis() - lastPrevention > 100) {
        smartDelay(1);
        lastPrevention = millis();
    }
}

void setup(void) {
    Serial.begin(9600);

    Serial.println(F("\n---\nStarting...\n"));
    delay(100);

    Serial.println(F("Configuring GPS..."));
    initGPS();
    Serial.println(F("GPS module configured"));

    pinMode(GT_PIN_BUZZER, OUTPUT);
    pinMode(GT_PIN_LED_RED, OUTPUT);
    pinMode(GT_PIN_LED_GREEN, OUTPUT);
    pinMode(GT_PIN_LED_BLUE, OUTPUT);

    resetLEDs();

    Serial.println(F("Initializing SD card"));

    if (!SD.begin()) {
        cardNotReady();
    } else {
        File myFile = SD.open("touch.dat", FILE_OPEN_MODE);
        boolean ready = myFile;
        myFile.close();
        if (ready) Serial.println(F("SD card is ready!")); else cardNotReady();
    }

    // TODO LED blinking
    digitalWrite(GT_PIN_LED_BLUE, LOW);

    Serial.println(F("Searching for location..."));

    // TODO load cached GPS data?

    while (!gpsValid()) {
        smartDelay(1000);
        while (gpsSerial.available()) {
            gps.encode((char) gpsSerial.read());
        }
    }
    Serial.println(F("Found valid location!"));

    snapshotGPS();

    char dir[9];
    // it has to be <= 8 chars!!!
    sprintf(dir, "%02d-%02d-%02d", last_date.day(), last_date.month(), last_date.year() - 2000);

    if (!SD.exists(dir) && !SD.mkdir(dir)) {
        Serial.println(F("Could not create dir!"));
        cardNotReady();
    }

    preventOverflow();

    sprintf(filename,
            "%s/%02d-%02d-%02d%s",
            dir, last_time.hour(), last_time.minute(), last_time.second(), ".gpx"
    );

    Serial.print(F("Will log to "));
    Serial.println(filename);

    preventOverflow();

    dataFile = SD.open(filename, FILE_OPEN_MODE);

    if (!dataFile) {
        cardNotReady();
    }

    dataFile.print(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
                     "<gpx version=\"1.1\" creator=\"ArduinoGpsTracker\" xmlns=\"http://www.topografix.com/GPX/1/1\" \r\n"
                     "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n"
                     "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\r\n"));
    char trk[30];
    sprintf(trk,
            "<trk><name>%s %02d:%02d:%02d</name>",
            dir, last_time.hour(), last_time.minute(), last_time.second()
    );
    dataFile.print(trk);
    dataFile.print(F("\r\n<trkseg>\r\n"));  // GPX header

    preventOverflow();

    dataFile.print(GPX_FOOTER);

    dataFile.flush();
    dataFile.close();

    preventOverflow();

    digitalWrite(GT_PIN_LED_BLUE, HIGH);
    digitalWrite(GT_PIN_LED_GREEN, LOW);

    Serial.println("\n");
}


void loop(void) {
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("ERROR: not getting any GPS data!"));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (true)beepAndBlinkRed(300);
#pragma clang diagnostic pop
    }

    // causes read of GPS data internally
    preventOverflow();

    boolean valid = gpsValid();

    if (valid) {
        snapshotGPS();

        digitalWrite(GT_PIN_LED_GREEN, LOW);
        digitalWrite(GT_PIN_LED_RED, HIGH);
        digitalWrite(GT_PIN_LED_BLUE, HIGH);

        writeToGpx();
    } else {
        // TODO check what happens
        Serial.println("\nLost location!\n");
        digitalWrite(GT_PIN_LED_BLUE, LOW);
        digitalWrite(GT_PIN_LED_GREEN, HIGH);
        digitalWrite(GT_PIN_LED_RED, HIGH);
    }

    smartDelay(GPX_INTERVAL);
}

void snapshotGPS() {
    last_location = gps.location;
    last_date = gps.date;
    last_time = gps.time;
    last_sats = gps.satellites;
    last_alti = gps.altitude;
    last_hdop = gps.hdop;
}

bool gpsValid() {
    Serial.println(F("---"));
    Serial.print(F("LOC: valid: "));
    Serial.print(gps.location.isValid() > 0 ? "true" : "false");
    Serial.print(F(" "));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F("/"));
    Serial.print(gps.location.lng(), 6);
    Serial.print(F(", ele: "));
    Serial.print(gps.altitude.value());
    Serial.print(F(", age: "));
    Serial.print(gps.location.age());
    preventOverflow();
    Serial.print(F(", date: "));
    Serial.print(gps.date.value());
    Serial.print(F(", sats: "));
    Serial.print(gps.satellites.value());
    Serial.print(F(", HDOP: "));
    Serial.print(gps.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)
    Serial.print(F(", fixes: "));
    Serial.print(gps.sentencesWithFix());
    Serial.print(F(", CRC failures "));
    Serial.println(gps.failedChecksum());

    preventOverflow();

    return gps.location.isValid() && gps.date.isValid() && gps.date.age() <= 1500;
}

void writeToGpx() {
    preventOverflow();

    byte hour = gps.time.hour();
    byte day = last_date.day();

    hour = hour + TIMEZONE; // TODO handle timezone
    if (hour > 23) {
        hour = hour - 24;
        day += 1;
    }
    char formattedDate[22];
    sprintf(formattedDate,
            "%4d-%02d-%02dT%02d:%02d:%02dZ",
            last_date.year(), last_date.month(), day, hour, last_time.minute(), last_time.second()
    );

    preventOverflow();

    dataFile = SD.open(filename, FILE_OPEN_MODE);

    unsigned long filesize = dataFile.size();
    // move the file pointer to just before the closing tags

    filesize -= GPX_FOOTER_LENGTH;
    dataFile.seek(filesize);

    preventOverflow();
    dataFile.print(F("<trkpt lat=\""));
    dataFile.print(last_location.lat(), GPX_LOC_ACCURACY);
    dataFile.print(F("\" lon=\""));
    dataFile.print(last_location.lng(), GPX_LOC_ACCURACY);
    dataFile.print(F("\">"));

    preventOverflow();
    dataFile.print(F("<time>"));
    dataFile.print(formattedDate);
    dataFile.print(F("</time>"));

    if (last_alti.isValid()) {
        preventOverflow();
        dataFile.print(F("<ele>"));
        dataFile.print(last_alti.meters(), 1);
        dataFile.print(F("</ele>"));
    }

    if (last_sats.isValid()) {
        preventOverflow();
        dataFile.print(F("<sat>"));
        dataFile.print(gps.satellites.value());
        dataFile.print(F("</sat>"));
    }

    if (last_hdop.isValid()) {
        preventOverflow();
        dataFile.print(F("<hdop>"));
        dataFile.print(gps.hdop.value(), 1);
        dataFile.print(F("</hdop>"));
    }

    dataFile.println(F("</trkpt>"));
    dataFile.print(GPX_FOOTER);
    dataFile.flush();
    dataFile.close();
    preventOverflow();
}

static void smartDelay(unsigned long ms) {
    unsigned long end = millis() + ms;
    do {
        while (gpsSerial.available())
            gps.encode((char) gpsSerial.read());
    } while (millis() < end);
}

static void beep(int length) {
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, HIGH);
    smartDelay(length);
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, LOW);
}

static void beepAndBlinkRed(int length) {
    resetLEDs();
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, HIGH);
    digitalWrite(GT_PIN_LED_RED, LOW);
    smartDelay(length);
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, LOW);
    digitalWrite(GT_PIN_LED_RED, HIGH);
}

static void beepAndBlinkGreen(int length) {
    resetLEDs();
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, HIGH);
    digitalWrite(GT_PIN_LED_GREEN, LOW);
    smartDelay(length);
    if (BUZZER_ENABLED) digitalWrite(GT_PIN_BUZZER, LOW);
    digitalWrite(GT_PIN_LED_GREEN, HIGH);
    smartDelay(10);
}

void resetLEDs() {
    digitalWrite(GT_PIN_LED_RED, HIGH);
    digitalWrite(GT_PIN_LED_GREEN, HIGH);
    digitalWrite(GT_PIN_LED_BLUE, HIGH);
}

void cardNotReady() {
    resetLEDs();

    Serial.println("Card failed, or not present!");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while (true) {
        beepAndBlinkRed(500);
        delay(200);
    }
#pragma clang diagnostic pop
}
