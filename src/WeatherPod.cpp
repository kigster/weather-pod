/**
 * Libraries used in this WeatherPod:
 *
 * 		Wire
 * 		Time
 * 		EEPROMEx
 * 		DHT
 * 		Adafruit_BMP085_Unified
 * 		Adafruit_Sensor          for pressure, temp, humidity
 * 		DS1307RTC                for clocl
 * 		LiquidCrystal_I2C2004V1  for display
 * 		SimpleTimer
 *
 */


#include <Arduino.h>
#include <SimpleTimer.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "SetTime.h"

#define DEGREE_SYMBOL (char)223

#define LOW_PRESSURE 1002
#define HIGH_PRESSURE 1020
#define PRESSURE_SENSOR_OFFSET  6
#define PRESSURE_RANGE_OFFSET 950

#define COLUMNS 20

const uint8_t pinDHT = 2;
DHT dht(pinDHT, DHTTYPE);
LiquidCrystal_I2C lcd(0x3F, COLUMNS, 4);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
SimpleTimer timer;

#define HOURS 24

uint16_t hourlyPressure[HOURS];
uint8_t hourlyPressureCompact[HOURS]; // for saving to EEMPROM,

typedef struct configStruct  {
    uint16_t pause;
    float temperatureDHTinF;
    float temperatureBMPinF;
    float pressureInches;
    float pressureMb;
    float humidityPercent;
} configType;

typedef struct displayStruct {
    char row[4][COLUMNS + 1];
} displayType;

char buffer[COLUMNS];
displayType display;
configType config = { 10 };


/**********************************************************************************
 *
 * Utilities
 */

void logHistory() {
    time_t t;
    Serial.print("Hourly History [\n");
    int start = 0;
    if (t = RTC.get()) {
        start = hour(t);
    }
    for (int i = 0; i < HOURS; i++) {
        int index = (i + start) % HOURS;
        sprintf(buffer, "\t%02dh -> %04d\n", index, hourlyPressure[index]);
        Serial.print(buffer);
    }
    Serial.println("]\n");
}

char *ftoa(char *a, double f, int precision) {
    long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};

    char *ret = a;
    long heiltal = (long)f;
    itoa(heiltal, a, 10);
    while (*a != '\0') a++;
    *a++ = '.';
    long desimal = abs((long)((f - heiltal) * p[precision]));
    itoa(desimal, a, 10);
    return ret;
}

/**********************************************************************************
 *
 * Display routines
 *
 */
time_t getTime() {
    time_t t;

    if (t = RTC.get()) {
        sprintf(display.row[0],
                "%02d/%02d/%4d  %02d:%02d:%02d",
                month(t),
                day(t),
                year(t),
                hour(t),
                minute(t),
                second(t)
               );

        // save the value for this hour
        hourlyPressure[hour(t)] = config.pressureMb;
    } else {
        if (RTC.chipPresent()) {
            setTime();
            sprintf(display.row[0], "Must init time chip!");
        } else {
            sprintf(display.row[0], "Time chip not found.");
        }
    }
    return t;
}

void displayReadings() {
    time_t t = getTime();
    char temp1[COLUMNS + 1];
    char temp2[COLUMNS + 1];
    char temp3[COLUMNS + 1];

    lcd.setCursor(0,0);

    ftoa(temp1, config.humidityPercent, 1);
    sprintf(display.row[1], "Humidity:      %s%%", temp1);

    ftoa(temp1, config.pressureMb, 1);
    // now check the difference in the last 3 hours
    int hourToCompare = hour(t) - 3; // 3 behind
    if (hourToCompare < 0)
        hourToCompare = 24 + hourToCompare; // handle midnight

    // reset hourly pressure when we boot.
    for (int i = 0; i < 24; i++ ) {
        if (hourlyPressure[i] == 0) {
            hourlyPressure[i] = config.pressureMb;
        }
    }

    float delta = config.pressureMb - hourlyPressure[hourToCompare];
    ftoa(temp2, abs(delta), 1);

    sprintf(temp3, "%s", (delta > 0 ? "+" : "-"));

    if (abs(delta) < 0.001 ) sprintf(temp3, " ");

    sprintf(display.row[2], "Air: [%s%s] %smb", temp3, temp2, temp1);

    //float tmp = (config.temperatureBMPinF + config.temperatureDHTinF) / 2.0;
    //	float tmp = config.temperatureDHTinF;
    float tmp = config.temperatureBMPinF;

    ftoa(temp3, 5.0 / 9.0 * (tmp - 32.0), 1);
    ftoa(buffer, tmp, 1);

    sprintf(display.row[3], "Temp:  %s%cF %s%cC", buffer,
            DEGREE_SYMBOL, temp3,
            DEGREE_SYMBOL);

    for (int i = 0; i < 4; i++) {
        lcd.setCursor(0,i);
        lcd.print(display.row[i]);
        Serial.println(display.row[i]);
    }
    Serial.println(F("====================================="));
}
/**********************************************************************************
 *
 * Sensor readings routines
 */

float hPa2inHg(float value) {
    return value * 2.954 * 0.01;
}

void readBMP() {
    sensors_event_t event;
    bmp.getEvent(&event);
    /* Display the results (barometric pressure is measure in hPa) */
    if (event.pressure) {
        config.pressureMb = event.pressure + PRESSURE_SENSOR_OFFSET;  // hPa
        config.pressureInches = (float) hPa2inHg((float) event.pressure);
        float temperature;
        bmp.getTemperature(&temperature);
        config.temperatureBMPinF = temperature * 9.0 / 5.0 + 32.0;
    } else {
        Serial.println("Sensor Error :(");
    }
}

void readDHT() {
    float h = dht.readHumidity();
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
    }

    config.humidityPercent = h;
    config.temperatureDHTinF = f;
}


/**********************************************************************************
 *
 * Arduino Callbacks
 */
void setup() {
    Serial.begin(9600); //Setup the debug terminal at regular 9600bps

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet)
        Serial.println(F("Unable to sync with the RTC"));
    else {
        Serial.println(F("RTC has set the system time"));
		}

    dht.begin();
    bmp.begin();
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);

    memset(&display, 0x0, sizeof(displayType));
    memset(&hourlyPressure, 0x0, sizeof(hourlyPressure)*sizeof(uint16_t));

    Serial.print(F("Resetting memory structures."));
    Serial.println(sizeof(displayType));
    Serial.println(sizeof(hourlyPressure) * sizeof(uint16_t));

    timer.setInterval(2000, &readBMP);
    delay(500);
    timer.setInterval(2000, &readDHT);
    delay(500);
    timer.setInterval(1000, &displayReadings);
}

void loop() {
    timer.run();
    delay(config.pause);
}
