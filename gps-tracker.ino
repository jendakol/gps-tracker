#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#define GT_PIN_SD_CS A1
#define GT_PIN_SD_SCK A2

#define GT_PIN_GPS_RX 4
#define GT_PIN_GPS_TX 3

#define GT_PIN_LED_G 11

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GT_PIN_GPS_RX, GT_PIN_GPS_TX);

char filename1[20];
char date1[22];
char filepath[20];
unsigned long start;
float latitude, longitude;
unsigned long speed;
int year, h_dop;
byte month, day, hour, minute, second;
File dataFile;

static void smartdelay(unsigned long ms);

void setup(void) {
    gpsSerial.begin(9600);
    Serial.begin(9600);

    pinMode(GT_PIN_LED_G, OUTPUT);

//    if (!SD.begin(SD_CS)) {
//        Serial.println("Card failed, or not present");
//    } else {
//        Serial.println("SD card is ready");
//    }

    if (gpsSerial.available()) {
        gps.encode((char) gpsSerial.read());
        h_dop = gps.hdop.value();
    }

    Serial.print("HDOP: ");
    Serial.println(h_dop);

//
//    do { Serial.println("wait");
//
//        smartdelay(1000);
//        h_dop=gps.hdop();
//        Serial.println(h_dop);
//        gps.f_get_position(&latitude, &longitude);
//        Serial.print(latitude);
//        Serial.print(" ");
//        Serial.println(longitude);
//    } while (h_dop ==  TinyGPS::GPS_INVALID_HDOP); //the track record does not start until gps fix
//
//    gps.crack_datetime(&year,&month,&day,&hour,&minute,&second);
//    hour = hour + 3;
//    if (hour > 23) {
//        hour = hour - 24;
//        day += 1;
//    }
//    sprintf(filename1, "/%02d-%02d-%02d", day, month, year - 2000);//название папки
//    sprintf(filepath, "/%02d-%02d-%02d/%02d-%02d%s", day, month, year - 2000, hour, minute, ".GPX");
//    //sprintf(filepath, "%02d-%02d%s",hour, minute, ".gpx");//путь к файлу в папке
//    Serial.println(filepath);
//    SD.mkdir(filename1);

}


void loop(void) {
    Serial.println("\n\nReading");


    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println("ERROR: not getting any GPS data!");
        // dump the stream to Serial
        Serial.println("GPS stream dump:");
        while (true) // infinite loop
            if (gpsSerial.available() > 0) // any data coming in?
                Serial.write(gpsSerial.read());
    }

//    Serial.print("Sentences that failed checksum=");
//    Serial.println(gps.failedChecksum());
//
//// Testing overflow in SoftwareSerial is sometimes useful too.
//    Serial.print("Soft Serial device overflowed? ");
//    Serial.println(gpsSerial.overflow() ? "YES!" : "No");



    while (gpsSerial.available())
        gps.encode((char) gpsSerial.read());

    if (gps.location.isValid()) {
        Serial.println("VALID!!!");
        digitalWrite(GT_PIN_LED_G, LOW);
    }

    Serial.print("Loc valid: ");
    Serial.println(gps.location.isValid());

    Serial.println(gps.location.lat(), 6); // Latitude in degrees (double)
    Serial.println(gps.location.lng(), 6); // Longitude in degrees (double)
//    Serial.print(gps.location.rawLat().negative ? "-" : "+");
//    Serial.println(gps.location.rawLat().deg); // Raw latitude in whole degrees
//    Serial.println(gps.location.rawLat().billionths);// ... and billionths (u16/u32)
//    Serial.print(gps.location.rawLng().negative ? "-" : "+");
//    Serial.println(gps.location.rawLng().deg); // Raw longitude in whole degrees
//    Serial.println(gps.location.rawLng().billionths);// ... and billionths (u16/u32)
    Serial.print("Date: ");
    Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
//    Serial.println(gps.date.year()); // Year (2000+) (u16)
//    Serial.println(gps.date.month()); // Month (1-12) (u8)
//    Serial.println(gps.date.day()); // Day (1-31) (u8)
//    Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
//    Serial.println(gps.time.hour()); // Hour (0-23) (u8)
//    Serial.println(gps.time.minute()); // Minute (0-59) (u8)
//    Serial.println(gps.time.second()); // Second (0-59) (u8)
//    Serial.println(gps.time.centisecond()); // 100ths of a second (0-99) (u8)
//    Serial.println(gps.speed.value()); // Raw speed in 100ths of a knot (i32)
//    Serial.println(gps.speed.knots()); // Speed in knots (double)
//    Serial.println(gps.speed.mph()); // Speed in miles per hour (double)
//    Serial.println(gps.speed.mps()); // Speed in meters per second (double)
//    Serial.println(gps.speed.kmph()); // Speed in kilometers per hour (double)
//    Serial.println(gps.course.value()); // Raw course in 100ths of a degree (i32)
//    Serial.println(gps.course.deg()); // Course in degrees (double)
//    Serial.println(gps.altitude.value()); // Raw altitude in centimeters (i32)
//    Serial.println(gps.altitude.meters()); // Altitude in meters (double)
//    Serial.println(gps.altitude.miles()); // Altitude in miles (double)
//    Serial.println(gps.altitude.kilometers()); // Altitude in kilometers (double)
//    Serial.println(gps.altitude.feet()); // Altitude in feet (double)
    Serial.print("Sats: ");
    Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)

//    bool isSignal = false;
//    if(h_dop !=  TinyGPSPlus::GPS_INVALID_HDOP)
//        isSignal = true;
//    else
//        return;
//
//    float falt, h_dop1 ;
//    gps.crack_datetime(&year,&month,&day,&hour,&minute,&second);
//    gps.f_get_position(&latitude, &longitude);
//    falt = gps.altitude()/100;
//    h_dop1 = gps.hdop();
//    h_dop1 = h_dop1 /100;
//    int spd = gps.f_speed_kmph();
//    hour = hour + 2; //set your time zone, my zone is +3
//    if (hour > 23) {hour = hour - 24; day +=1;}
//    year -= 2000;
//    sprintf(date1, "%4d-%02d-%02dT%02d:%02d:%02dZ",year, month, day, hour,minute,second);

//    if (!SD.exists(filepath)) {
//        dataFile = SD.open(filepath, FILE_WRITE);
//
//        dataFile.print(F("<trkpt lat=\""));
//        dataFile.print(latitude);
//        dataFile.print(F("\" lon=\""));
//        dataFile.print(longitude);
//        dataFile.println(F("\">"));
//        dataFile.print(F("<time>"));
//        dataFile.print(date1);
//        dataFile.println(F("</time>"));
//        dataFile.println(F("</trkpt>"));
//        dataFile.close();
//    }
//
//    smartdelay(1000);
}

static void smartdelay(unsigned long ms) {
    start = millis();
    do {
        while (gpsSerial.available())
            gps.encode((char) gpsSerial.read());
    } while (millis() - start < ms);
}

