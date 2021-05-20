/************************************************************************
  Sketch: OLed_DS18B20
  
  Programmer: Ashish Pardeshi
  
  Description: This sketch will read Temperature (from DS18B20 sensor)
               DateTime Stamp from RTC (DS3231).
               It will display data on OLED.
               
  Interfacings
  --------------------------------------------------------------------------------
  S.No            Device            Parameters                Peripherals/Pins
  
  1               DS18B20           Temperature &             IO19
                                    
 
  2               OLED              Display Module            I2C (SDA, SCL)
                  128 x 64   
                  
  4               Buzz_IO           Buzzer                    IO25
  
  5               BoardLED          ON Board_LED              IO14
  --------------------------------------------------------------------------------
*/


#include <Wire.h>

#include <OneWire.h>
#include <DallasTemperature.h>


#include <Adafruit_Sensor.h>

#include <Adafruit_SSD1306.h>

#include <ESP32_tone.h>

#include "DS3231.h"

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3
};

int noteDurations[] = {
  2, 4, 8, 8, 2
};

int buzzerPin = 25;
//create an instance named buzzer
ESP32_tone buzzer;



//#define DEVICE_ID 1     // 0 for CLAB Server and 1 for Makerspace room

#define BUZZER  25
#define BOARD_LED  14 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)


#define DS18B20PIN 19     // DS18b20 data pin connected to IO19 of ESP32

/* Create an instance of OneWire */
OneWire oneWire(DS18B20PIN);

DallasTemperature ds18b20(&oneWire);

// Creating objects for sensors/modules  
RTClib RTC; 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



// ******** Global Variables *********

volatile float temperature;

String mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", 
                  "Jun", "Jul", "Aug", "Sep", "Oct", 
                  "Nov", "Dec"
                 };


const unsigned char temprature_bmp [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 
  0xff, 0x00, 0x3f, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xfc, 0x3e, 0x1f, 0xff, 0xff, 0xfc, 
  0x7f, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0x8f, 0xff, 0xff, 0xf8, 0xff, 0x8f, 0xff, 0xff, 0xf8, 0xff, 
  0xc7, 0xff, 0xff, 0xf8, 0xfc, 0x07, 0xff, 0xff, 0xf8, 0xf8, 0x07, 0xff, 0xff, 0xf8, 0xfc, 0x07, 
  0xff, 0xff, 0xf8, 0xff, 0xc7, 0xff, 0xff, 0xf8, 0xff, 0x87, 0xff, 0xff, 0xf8, 0xfc, 0x07, 0xff, 
  0xff, 0xf8, 0xf8, 0x07, 0xff, 0xff, 0xf8, 0xfc, 0x07, 0xff, 0xff, 0xf8, 0xff, 0x87, 0xff, 0xff, 
  0xf8, 0xff, 0xc7, 0xff, 0xff, 0xf8, 0xfc, 0x07, 0xff, 0xff, 0xf8, 0xf8, 0x07, 0xff, 0xff, 0xf8, 
  0xfc, 0x07, 0xff, 0xff, 0xf8, 0xff, 0xc7, 0xff, 0xff, 0xf8, 0xff, 0xc7, 0xff, 0xff, 0xf8, 0xf3, 
  0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 
  0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 
  0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 0xf8, 0xe1, 0xc7, 0xff, 0xff, 
  0xe0, 0xe1, 0xc3, 0xff, 0xff, 0xc1, 0xc1, 0xc1, 0xff, 0xff, 0x83, 0xc0, 0xf0, 0xff, 0xff, 0x87, 
  0x80, 0xf8, 0x7f, 0xff, 0x0f, 0x00, 0x3c, 0x7f, 0xff, 0x1e, 0x00, 0x1c, 0x3f, 0xfe, 0x1c, 0x00, 
  0x0e, 0x3f, 0xfe, 0x38, 0x00, 0x0e, 0x1f, 0xfe, 0x38, 0x00, 0x07, 0x1f, 0xfc, 0x38, 0x00, 0x07, 
  0x1f, 0xfc, 0x70, 0x00, 0x07, 0x1f, 0xfc, 0x70, 0x00, 0x07, 0x1f, 0xfc, 0x70, 0x00, 0x07, 0x1f, 
  0xfc, 0x38, 0x00, 0x07, 0x1f, 0xfe, 0x38, 0x00, 0x07, 0x1f, 0xfe, 0x38, 0x00, 0x0e, 0x1f, 0xfe, 
  0x1c, 0x00, 0x0e, 0x3f, 0xff, 0x1e, 0x00, 0x1c, 0x3f, 0xff, 0x0f, 0x00, 0x3c, 0x7f, 0xff, 0x87, 
  0xc0, 0xf8, 0x7f, 0xff, 0xc3, 0xff, 0xe0, 0xff, 0xff, 0xe0, 0xff, 0xc1, 0xff, 0xff, 0xf0, 0x1e, 
  0x03, 0xff, 0xff, 0xf8, 0x00, 0x0f, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xff, 0x80, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};



void setup() 
{
  // initialize serial for debugging
  Serial.begin(9600);

  pinMode(BOARD_LED,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  
  digitalWrite(BUZZER,LOW);
  digitalWrite(BOARD_LED,LOW);

   //initialize buzzer
  buzzer.ESP32_toneB(0);

  //set the compatibleMode of the library
  buzzer.setCompatibleMode(true);


  // -----------OLED Initialization ------------------------------
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }
  //--------------------------------------------------------------

  // ****** Welcome Note ********************
  intro_melody();
  
  display.clearDisplay();
  display.display();
  
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(5, 5);
  //display.println(F("Temp & Hum"));
  display.println(F("  Oxygen  "));
  display.setCursor(0, 25);
  display.println(F(" Analyzer "));
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 43);
  display.println(F(" Monitoring and Alert"));
  display.setCursor(0, 56);
  display.println(F("       Device        "));
  display.display();
  delay(2000);
  // ********************************************************************


  
  // ************* O2 Sensor Startup Time (12 Seconds) ******************
  display.clearDisplay();
  display.display();
  
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println(F(" O2 Startup "));
  display.setCursor(30, 50);
  display.println(F(" Time Starts "));
  display.display();
  
  Serial.println("O2 Startup Time");
  delay(12000);   // 12 Secs Startup Time 
  Serial.println("O2 Startup Done");
  
  display.clearDisplay();
  display.display();
  
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println(F(" O2 Startup"));
  display.setCursor(30, 50);
  display.println(F("    Done   "));
  display.display();
  
  // ********************************************************************
  Serial.println("Setup Done");

  // ******** Sensor Initialization *******************************************
  ds18b20.begin();
  Wire.begin();

  Serial.println  ("Sensors Done");
}

void loop() 
{
  getDS18B20Readings();       // Read Temperature
  oledTemp();
}



void getDS18B20Readings()
{
  ds18b20.requestTemperatures(); 
  temperature = ds18b20.getTempCByIndex(0);

  //messageTH = "Temperature: " + String(temperature) + " ºC \n";
}



void oledTemp()
{
  display.clearDisplay();
  oledDataTime();
  display.drawBitmap(0, 0,temprature_bmp, 40, 64, WHITE); 

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(53, 25);
  display.print(temperature);
  display.setTextSize(1); // Draw 2X-scale text
  display.setCursor(52, 50);
  display.println(F("Deg Celcius"));
  display.display();
  
  delay(500);
}

void oledDataTime()
{
  DateTime now = RTC.now();

  String dateTimeStamp;
  dateTimeStamp = String(now.day()) + " " + String(mon[now.month() - 1]) + ", ";
  dateTimeStamp += String(now.hour())+ ":" + String(now.minute());
  //dateTimeStamp = "26 Jan, 20:59";
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(50, 5);
  Serial.println(dateTimeStamp);
  display.println(dateTimeStamp);
}

void buzzBeep()
{
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
}

void buzzBeepInd()
{
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
}

/*void buzzAlert()
{
  if(temperature >= maxTemp || humidity >= maxHumidity || smoke >= maxSmoke)
  {
    buzzBeep();
  } 
}*/



void intro_melody()
{
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    buzzer.tone(25, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    buzzer.noTone(25);
  }
}
