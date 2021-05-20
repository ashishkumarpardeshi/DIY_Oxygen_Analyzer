/************************************************************************
  Sketch: OLed_Display_OTA
  
  Programmer: Ashish Pardeshi
  
  Description: This sketch will read Temperature (from DS18B20 sensor)
               DateTime Stamp from RTC (DS3231).
               It will display data on OLED.
               It will also contain a piece of code for OTA programming.
               
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


#ifdef ESP32

  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiClientSecure.h>

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

const char* host = "esp32";

/* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex = 
"<form name=loginForm>"
"<h1>Makerspace (121) MAD Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='makerspace' && form.pwd.value=='rocks')"
"{window.open('/serverIndex')}"
"else"
"{alert('Error Password or Username')}"
"}"
"</script>" + style;
 
/* Server Index Page */
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;


WebServer server(80);

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
//Adafruit_BME280 bme;     
RTClib RTC; 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



//WiFiClientSecure client;
//UniversalTelegramBot bot(BOTtoken, client);

String macAddr;


// ******** Global Variables *********

char* ssid;//[] = { };

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


  // **************** Welcome Note **************************************
  
  display.clearDisplay();
  display.display();
  
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(5, 5);
  display.println(F("Makerspace"));
  display.setCursor(0, 25);
  display.println(F("    DIY    "));
  display.setCursor(0, 45);
  display.println(F("  Culture "));
  display.display();
 
  Serial.println("Welcome");
  
  delay(3000);


  // ****** Welcome Melody ********************
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
  delay(3000);
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

  // ********** Setting for OTA Programming *****************************
  // Set WiFi to station mode and 
  // disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup Done");

  InitWiFi();

  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() 
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  
  server.on("/serverIndex", HTTP_GET, []() 
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() 
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  // **************************************************************************

  // ******** Sensor Initialization *******************************************
  ds18b20.begin();
  Wire.begin();

  Serial.println  ("Sensors Done");
}

void loop() 
{
  server.handleClient();      // OTA Prgram Check API

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

void InitWiFi()
{
  Serial.println("Initialising WiFi");
  WiFi.mode(WIFI_STA);
  
  printMacAddress(); 
  
  AP_Connection();  
}

void AP_Connection()
{
  scanNetwork();

  int connect_count = 0;
  
  String ssid = WiFi.SSID(0);
  int ssid_len = ssid.length() + 1;
  char ssid_charArray[ssid_len];
  ssid.toCharArray(ssid_charArray, ssid_len);
  Serial.print("ssid CharArray[0]: ");
  
  Serial.println("Attemp to connect SSID: ");
  Serial.println(ssid_charArray);

  while ( WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("..");
    WiFi.begin(ssid_charArray);
    connect_count ++;
    if(connect_count == 3)
    {
      ssid = WiFi.SSID(1);
      int ssid_len = ssid.length() + 1;
      char ssid_charArray[ssid_len];
      ssid.toCharArray(ssid_charArray, ssid_len);
      Serial.print("ssid CharArray[1]: ");
    }
    if(connect_count == 6)
    {
      ssid = WiFi.SSID(2);
      int ssid_len = ssid.length() + 1;
      char ssid_charArray[ssid_len];
      ssid.toCharArray(ssid_charArray, ssid_len);
      Serial.print("ssid CharArray[2]: ");
    }

    if(connect_count > 8)
    {
      ESP.restart();
    }
  }
  
  Serial.print("Connected to AP: ");
  Serial.println(ssid_charArray);
  
  Serial.println("Obtaining IP: ");
  IPAddress ip = WiFi.localIP();
  //String ipS = String(ip);
  //int ip_len = strlen(ip) + 1;
  //char ip_charArray[ip_len];
  /*for(int i = 0; i<=ip_len,i++)
  {
    ip_charArray[i] = ip[i];
  }*/
  
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  display.clearDisplay();
  display.display();
  
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("Connected to AP: "));
  display.setCursor(0,15);
  display.println(ssid_charArray);
  display.setCursor(0,30);
  display.println(macAddr);
  display.setTextSize(2);
  display.setCursor(0, 50);
  display.println(ip);
  display.display();

  delay(4000);

  buzzBeepInd();

  /*String ipAdd = WiFi.localIP();
  int ip_len = ipAdd.length() + 1;
  char ip_charArray[ip_len];
  ip.toCharArray(ip_charArray, ip_len);*/

  Serial.println("initialization Done");

  delay(50);


    if(WiFi.status() == WL_CONNECTED) 
    {
      Serial.println("connected");
    }
}

void printMacAddress()
{
  // get your MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  
  // print MAC address
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  Serial.print("MAC address: ");
  Serial.println(buf);
  macAddr = "MAC "+ String(buf); //+ mac[5] + ":" + mac[4] + ":" + mac[3] + ":" + mac[2] + ":" + mac[1] + ":" + mac[0];
}

void scanNetwork()
{
  Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) 
    {
        Serial.println("no networks found");
    } 
    else 
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) 
        {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
}

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
