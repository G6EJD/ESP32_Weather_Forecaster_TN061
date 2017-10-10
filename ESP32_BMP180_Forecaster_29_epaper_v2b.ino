/*########################   Zambretti Weather Forecaster #############################
 * Forecasts the weather using the Zambretti algorithm, displays weather symbol, weather description, air pressure and pressure trend
 * This source code is protected under the terms of the MIT License and is copyright (c) 2017 by David Bird and permission is hereby granted, free of charge, to
 * any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, but not to sub-license and/or 
 * to sell copies of the Software or to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *   FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 *   ANY REVENUE FROM THE SOFTWARE MUST BE APPORTIONED TO THE COPYRIGHT OWNER D.L.BIRD (C) 2017
 *   See more at http://dsbird.org.uk
 *   
 *   Display used was Waveshare 1.54" e-Ink, powered used was 5mWhr over 9-hours, or 0.55mW every hour and at 5v = 0.11mA
 *   WEMOS ESP32 Lite consumes 1.88mA in sleep mode powered directly by the 3v3 pin
 *   WEMOS ESP32 Lite consumes 5.65mA in sleep mode powered directly by the 5v USB
 *   A 2600mAhr battery would last for 2600mAhr/0.11mA = 2.67years!
 */   
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <GxEPD.h>
//#include <GxGDEP015OC1/GxGDEP015OC1.cpp>
#include <GxGDEW042T2B/GxGDEW042T2B.cpp> // 2.9" driver a modified 4.2" driver!
#include <Fonts/FreeSans9pt7b.h>
#include "credentials.h"
#include <SPIFFS.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

// pins_arduino.h, e.g. LOLIN32 LITE
//static const uint8_t SS    = 5;
//static const uint8_t MOSI  = 23;
//static const uint8_t MISO  = 19;
//static const uint8_t SCK   = 18;

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, 5, 17, 16); // arbitrary selection of 17, 16
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io, 16, 4); // arbitrary selection of (16), 4

//###########################################################################
String  time_str, wx_text, wx_image, trend_img;
int8_t  last_reading_hour, reading_hour, hr_cnt;
int16_t long_trend;
float   temperature;  

// An array structure to record pressure, temperaturre, humidity and weather state
float reading[24]; // An array covering 24-hours to enable P, T, % and Wx state to be recorded for every hour

Adafruit_BMP085 bmp;
#define screen_rotation 1
#define pressure_offset 6.7 // Used to adjust sensor reading to correct pressure for your location
int SLEEPTIME = 60*60; // 1-hr

///////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  StartWiFi(ssid, password);
  SetupTime();
  Wire.begin(13,15); // (sda,scl) // A BMP180 sensor on SDA=13 SCL=15
  if (!bmp.begin()) {
    Serial.println("Could not find a sensor, check wiring!");
  }
  else
  {
    Serial.println("Found a sensor continuing");
    while (isnan(bmp.readPressure())) {
      Serial.println(bmp.readPressure());
    }
  }
  if (!SPIFFS.begin()) { 
    Serial.println("SPIFFS Mount Failed"); 
    return; 
  } 
  Initialise_Display();
  Initialise_Variables();
  Read_Variables_from_SPIFFS();
  update_time_and_data();
  drawFrame1(0, 0);
  drawFrame2(220, 50);
  //drawFrame3(0, 100);
  display.update();
  Save_Variables_to_SPIFFS();
  ESP.deepSleep(SLEEPTIME*1000000); // 1,000,000uS=1-sec, then 60-secs*60-mins = 1-hour
}

void loop() {
  // Do nothing as this will never be reached
}

// This frame draws a weather icon based on 3-hours of data for the prediction
void drawFrame1(int16_t x, int16_t y) {
  float trend = reading[23] - reading[20];             // Trend over the last 3-hours
  calc_zambretti(reading[20], get_trend_text(trend));  // From forecast and trend determine what image to display
  DisplayWXicon(x + 30, y + 95, wx_image);
  DisplayTrend(x+145,y,trend_img);
  display.setCursor(x + 0, y + 12); display.println(String(float(reading[23]), 1) + " hPa ("+String(trend, 1)+")");
  display.setCursor(x + 170, y + 12); display.println(String(temperature,1)+" C");
  display.drawCircle(x+208,y+2,2,GxEPD_BLACK);
  display.setCursor(x + 0, y + 32); display.println(wx_text);
}

// This frame draws a graph of pressure (delta) change for the last 24-hours, see Annex* for more details
void drawFrame2(int16_t x, int16_t y) {
  int gwidth   = 60; // Graph width in pixels
  int gscale   = 50; // Graph height in pixels
  int num_bars = 8;  // Number of bars to display
  #define yscale 8   // Graph +/- y-axis scale  e.g. 8 displays +/-8 and scales data accordingly
  float bar_width = gwidth / (num_bars + 1); // Determine bar width based on graph width
  display.setFont();
  display.drawLine(x-1,y,x-1,y+gscale,GxEPD_BLACK);
  display.drawLine(x,y+gscale/2,x+gwidth+bar_width*2,y+gscale/2,GxEPD_BLACK);
  display.setCursor(x - 20, y-4);                                      display.println(">+" + String(yscale));
  display.setCursor(x - 8,  y-4 + gscale / 2);                         display.println("0");
  display.setCursor(x - 20, y-4 + gscale);                             display.println("<-" + String(yscale));
  display.setCursor(x + 1 + (bar_width + 3) * 0, y + gscale * 1.05); display.println("-24");
  display.setCursor(x + 1 + (bar_width + 3) * 7, y + gscale * 1.05); display.println("0");
  int display_points [8] = {0, 5, 11, 17, 20, 21, 22, 23}; // Only display time for hours 0,5,11,17,20,21,22,23
  float value;
  for (int bar_num = 0; bar_num < num_bars; bar_num++) {     // Now display a bar at each hour position -24, -18, -12, -6, -3, -2, -1 and 0 hour
    value = map(reading[display_points[bar_num]], reading[23] - yscale, reading[23] + yscale, gscale, 0);
    if (value > gscale) value = gscale;                      // Screen scale is 0 to e.g. 40pixels, this stops drawing beyond graph bounds
    if (value < 0     ) value = 0;                           // 0 is top of graph, this stops drawing beyond graph bounds
    display.drawFastHLine(x + bar_num * (bar_width + 3) + 2, y + value, bar_width, GxEPD_BLACK);
    for (int yplus = gscale; yplus > value; yplus = yplus - 1) {
      display.drawFastHLine(x + bar_num * (bar_width + 3) + 2, y + yplus, bar_width, GxEPD_BLACK);
    }
  }
  display.setCursor(x - 150, y + gscale * 1.4); display.println("(Updated: "+time_str+")");
  display.setFont(&FreeSans9pt7b);
  display.setCursor(x - 30 + (bar_width + 3) * 0, y + gscale * 1.5); display.println(String(hr_cnt / 24) + (hr_cnt/24<2?" Day ":" Days ") + String(hr_cnt % 24) + "Hr");
}

float read_pressure() {
  float pressure  = bmp.readPressure() / 100.0F + pressure_offset; 
  Serial.println(pressure);
  int altitude    = 40; // in METRES
  temperature = (int)bmp.readTemperature();
  return (double)pressure*pow(1-0.0065*(double)altitude/(temperature+0.0065*(double)altitude+273.15),-5.275);
}

void DisplayTrend(int x, int y, String trend_img){
  #define rising   1
  #define steady   0
  #define falling -1
  if      (trend_img == "steady")   draw_arrow(x, y+1, steady,  10); // x,y, direction, scale
  else if (trend_img == "srising")  draw_arrow(x, y+1, rising,  10);
  else if (trend_img == "rising")   draw_arrow(x, y+1, rising,  10);
  else if (trend_img == "sfalling") draw_arrow(x, y+1, falling, 10);
  else if (trend_img == "falling")  draw_arrow(x, y+1, falling, 10);
}

// Convert pressure trend to text
String get_trend_text(float trend) {
  String trend_str = "Steady"; // Default weather state
  if      (trend > 3.5)                    {trend_str = "Rising fast";  trend_img = "rising";}
  else if (trend > 1.5   && trend <= 3.5)  {trend_str = "Rising";       trend_img = "rising";}
  else if (trend > 0.25  && trend <= 1.5)  {trend_str = "Rising slow";  trend_img = "srising";}
  else if (trend > -0.25 && trend < 0.25)  {trend_str = "Steady";       trend_img = "steady";}
  else if (trend >= -1.5 && trend < -0.25) {trend_str = "Falling slow"; trend_img = "sfalling";}
  else if (trend >= -3.5 && trend < -1.5)  {trend_str = "Falling";      trend_img = "falling";}
  else if (trend <= -3.5)                  {trend_str = "Falling fast"; trend_img = "falling";}
  return trend_str;
}

String wx_forecast(char code){
  String wx_descriptor = "";
  switch (code) {
  case 'A': wx_descriptor = "Settled Fine Weather"; break;
  case 'B': wx_descriptor = "Fine Weather"; break;
  case 'C': wx_descriptor = "Becoming Fine"; break;
  case 'D': wx_descriptor = "Fine, Becoming Less Settled"; break;
  case 'E': wx_descriptor = "Fine, Possibly showers"; break;
  case 'F': wx_descriptor = "Fairly Fine, Improving"; break;
  case 'G': wx_descriptor = "Fairly Fine, Possibly showers early"; break;
  case 'H': wx_descriptor = "Fairly Fine, Showers Later"; break;
  case 'I': wx_descriptor = "Showery Early, Improving"; break;
  case 'J': wx_descriptor = "Changeable Improving"; break;
  case 'K': wx_descriptor = "Fairly Fine, Showers likely"; break;
  case 'L': wx_descriptor = "Rather Unsettled Clearing Later"; break;
  case 'M': wx_descriptor = "Unsettled, Probably Improving"; break;
  case 'N': wx_descriptor = "Showery Bright Intervals"; break;
  case 'O': wx_descriptor = "Showery Becoming Unsettled"; break;
  case 'P': wx_descriptor = "Changeable some rain"; break;
  case 'Q': wx_descriptor = "Unsettled, short fine Intervals"; break;
  case 'R': wx_descriptor = "Unsettled, Rain later"; break;
  case 'S': wx_descriptor = "Unsettled, rain at times"; break;
  case 'T': wx_descriptor = "Very Unsettled, Finer at times"; break;
  case 'U': wx_descriptor = "Rain at times, Worse later"; break;
  case 'V': wx_descriptor = "Rain at times, becoming very unsettled"; break;
  case 'W': wx_descriptor = "Rain at Frequent Intervals"; break;
  case 'X': wx_descriptor = "Very Unsettled, Rain"; break;
  case 'Y': wx_descriptor = "Stormy, possibly improving"; break;
  case 'Z': wx_descriptor = "Stormy, much rain"; break;
   default: wx_descriptor = "Unknown"; break;
  }
  return wx_descriptor;
}

void calc_zambretti(float zpressure, String ztrend) {
  int zmonth = GetMonth();
  // FALLING
  if (ztrend == "Falling" || ztrend == "Falling slow" || ztrend == "Falling fast") {
    double zambretti = 0.0009746*zpressure*zpressure - 2.1068*zpressure+1138.7019; //y = 0.0009746x^2-2.1068x+1138.7019
    // A Winter falling generally results in a Z value higher by 1 unit. 
    if (zmonth < 4 || zmonth > 9) zambretti = zambretti + 1; // + makes the forecast worst, - better!
    Serial.println(round(zambretti));
    switch (int(round(zambretti))) {
      case 1:  wx_text = wx_forecast('A'); wx_image = "sunny"; break;       //Settled Fine
      case 2:  wx_text = wx_forecast('B'); wx_image = "sunny"; break;       //Fine Weather
      case 3:  wx_text = wx_forecast('D'); wx_image = "expectrain"; break;  //Fine Becoming Less Settled
      case 4:  wx_text = wx_forecast('H'); wx_image = "expectrain"; break;  //Fairly Fine Showers Later
      case 5:  wx_text = wx_forecast('O'); wx_image = "expectrain"; break;  //Showery Becoming unsettled
      case 6:  wx_text = wx_forecast('R'); wx_image = "rain"; break;        //Unsettled, Rain later
      case 7:  wx_text = wx_forecast('U'); wx_image = "rain"; break;        //Rain at times, worse later
      case 8:  wx_text = wx_forecast('V'); wx_image = "rain"; break;        //Rain at times, becoming very unsettled
      case 9:  wx_text = wx_forecast('X'); wx_image = "rain"; break;        //Very Unsettled, Rain
      default: wx_text = "unknown";        wx_image = "nodata"; break;
    }
  }
  // STEADY
  if (ztrend == "Steady") {
    float zambretti = 138.24-0.133*zpressure; // y = 138.24-0.1331x
    switch (int(round(zambretti))) {
      case 1:  wx_text = wx_forecast('A'); wx_image = "sunny"; break;       //Settled Fine
      case 2:  wx_text = wx_forecast('B'); wx_image = "sunny"; break;       //Fine Weather
      case 3:  wx_text = wx_forecast('E'); wx_image = "expectrain"; break;  //Fine, Possibly showers
      case 4:  wx_text = wx_forecast('K'); wx_image = "expectrain"; break;  //Fairly Fine, Showers likely
      case 5:  wx_text = wx_forecast('N'); wx_image = "expectrain"; break;  //Showery Bright Intervals
      case 6:  wx_text = wx_forecast('P'); wx_image = "cloudy"; break;      //Changeable some rain
      case 7:  wx_text = wx_forecast('S'); wx_image = "rain"; break;        //Unsettled, rain at times
      case 8:  wx_text = wx_forecast('W'); wx_image = "rain"; break;        //Rain at Frequent Intervals
      case 9:  wx_text = wx_forecast('X'); wx_image = "rain"; break;        //Very Unsettled, Rain
      case 10: wx_text = wx_forecast('Z'); wx_image = "tstorms"; break;     //Stormy, much rain
      default: wx_text = "unknown";        wx_image = "nodata"; break;
    }
  }
  // RISING
  if (ztrend == "Rising" || ztrend == "Rising slow" || ztrend == "Rising fast") {
    float zambretti = 142.57-0.1376*zpressure; //y = 142.57-0.1376x
    //A Summer rising, improves the prospects by 1 unit over a Winter rising
    if (zmonth < 4 || zmonth > 9) zambretti = zambretti + 1; // Increasing values makes the forecast worst!
    switch (int(round(zambretti))) {
      case 1:  wx_text = wx_forecast('A'); wx_image = "sunny"; break;       //Settled Fine
      case 2:  wx_text = wx_forecast('B'); wx_image = "sunny"; break;       //Fine Weather
      case 3:  wx_text = wx_forecast('C'); wx_image = "mostlysunny"; break; //Becoming Fine
      case 4:  wx_text = wx_forecast('F'); wx_image = "mostlysunny"; break; //Fairly Fine, Improving
      case 5:  wx_text = wx_forecast('G'); wx_image = "expectrain"; break;  //Fairly Fine, Possibly showers, early
      case 6:  wx_text = wx_forecast('I'); wx_image = "expectrain"; break;  //Showery Early, Improving
      case 7:  wx_text = wx_forecast('J'); wx_image = "cloudy"; break;      //Changeable, Improving
      case 8:  wx_text = wx_forecast('L'); wx_image = "cloudy"; break;      //Rather Unsettled Clearing Later
      case 9:  wx_text = wx_forecast('M'); wx_image = "cloudy"; break;      //Unsettled, Probably Improving
      case 10: wx_text = wx_forecast('Q'); wx_image = "mostlysunny"; break; //Unsettled, short fine Intervals
      case 11: wx_text = wx_forecast('T'); wx_image = "cloudy"; break;      //Very Unsettled, Finer at times
      case 12: wx_text = wx_forecast('Y'); wx_image = "tstorms"; break;     //Stormy, possibly improving
      case 13: wx_text = wx_forecast('Z'); wx_image = "tstorms"; break;     //Stormy, much rain
      default: wx_text = "unknown";        wx_image = "nodata"; break;
    }
  }
  Serial.println(wx_text);
}

int StartWiFi(const char* ssid, const char* password) {
  int connAttempts = 0;
  Serial.print(F("\r\nConnecting to: ")); Serial.println(String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.print(".");
    if (connAttempts > 20) return -5;
    connAttempts++;
  }
  Serial.print(F("WiFi connected at: "));
  Serial.println(WiFi.localIP());
  return 1;
}

void SetupTime(){
  configTime(0, 0, "0.uk.pool.ntp.org", "time.nist.gov");
  setenv("TZ", "GMT0BST,M3.4.0/01,M10.4.0/02",1);
  delay(1000);
  UpdateLocalTime();
}

void UpdateLocalTime(){
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  //See http://www.cplusplus.com/reference/ctime/strftime/
  Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S"); // Displays: Saturday, June 24 2017 14:05:49
  char output[30];
  strftime(output, 30, "%H:%M:%S", &timeinfo);
  time_str = output;
}

int GetMonth() {
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  char output[30];
  strftime(output, 30, "%m", &timeinfo);
  String mon = output;
  return mon.toInt();  
}

void update_time_and_data() {
  if ((time_str.substring(0,2)).toInt() != reading_hour) { //After a restart the time may be the same so don't update anything
    reading_hour++;
    hr_cnt++;
    if (reading_hour > 24) reading_hour = 0;
    if (reading_hour != last_reading_hour) { // If the hour has advanced, then shift readings left and record new values at array element [23]
      for (int i = 0; i < 23; i++) {
        reading[i] = reading[i + 1];
      }
      reading[23]       = read_pressure(); // Update time=now with current pressure
      last_reading_hour = reading_hour;
    }
  }
}

void DisplayWXicon(int x, int y, String image_name) {
  int scale = 10;
  if      (image_name == "rain")         Rain(x,y,scale);
  else if (image_name == "sunny")        Sunny(x,y,scale);
  else if (image_name == "mostlysunny")  MostlySunny(x,y,scale);
  else if (image_name == "mostlycloudy") MostlyCloudy(x,y,scale);
  else if (image_name == "expectrain")   ExpectRain(x,y,scale);
  else if (image_name == "cloudy")       Cloudy(x,y,scale);
  else if (image_name == "snow")         Snow(x,y,scale);
  else if (image_name == "tstorms")      Tstorms(x,y,scale);
  else                                   Nodata(x,y,scale);
}

void Initialise_Display() {
  display.init();
  display.setRotation(screen_rotation);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
}

void Initialise_Variables() {
  reading_hour   = 0;
  hr_cnt         = 0;
  float pressure = read_pressure();
  for (int i = 0; i <= 23; i++) {   // At the start all array values are the same as a baseline
    reading[i] = pressure;
  }                                 // Note that only 0,5,11,17,20,21,22,23 are used as display positions
  last_reading_hour = reading_hour;
}

void Read_Variables_from_SPIFFS() {
  Initialise_Variables();
  char filename [] = "/data.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  File myDataFile = SPIFFS.open(filename, "r");        // Open a file for reading and writing (appending)
  if (!myDataFile) {
    Serial.println("Failed to open file");
  }
  String temp_data;
  temp_data = myDataFile.readStringUntil('\n');  hr_cnt = temp_data.toInt();
  temp_data = myDataFile.readStringUntil('\n');  reading_hour = temp_data.toInt();
  temp_data = myDataFile.readStringUntil('\n');  last_reading_hour = temp_data.toInt();
  for (int i = 0; i <= 23; i++) {
    temp_data = myDataFile.readStringUntil('\n');
    reading[i] = temp_data.toFloat();
  }
  myDataFile.close();
}

void Save_Variables_to_SPIFFS() {
  char filename [] = "/data.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  File myDataFile = SPIFFS.open(filename, "w");        // Open a file for reading and writing (appending)
  if (!myDataFile) {
    Serial.println("Failed to open file");
  }
  myDataFile.println(hr_cnt);
  myDataFile.println(reading_hour);
  Serial.println("Reading hour="+String(reading_hour));
  myDataFile.println(last_reading_hour);
  for (int i = 0; i <= 23; i++) {
    myDataFile.println(reading[i]);
  }
  myDataFile.close();
  Serial.println("Now reading back after write");
  myDataFile = SPIFFS.open(filename, "r");        // Open a file for reading and writing (appending)
  while (myDataFile.available()) {
    Serial.print("R="); Serial.println(myDataFile.readStringUntil('\n'));
  }
  myDataFile.close();
}

void draw_arrow(int x, int y, int dir, int scale){ //11h x 10w grid
  #define linesize 2
  if (dir== -1){     // Falling
    display.fillRect(x+scale, y+scale/2, linesize, scale/2+linesize, GxEPD_BLACK); // h x w
    display.fillRect(x+scale/2, y+scale, scale/2+linesize, linesize, GxEPD_BLACK);
    display.drawLine(x,y,x+scale,y+scale,GxEPD_BLACK);
    display.drawLine(x+1,y,x+scale+1,y+scale,GxEPD_BLACK);
  }
  else if (dir== 0){ // Steady
    display.fillRect(x, y+scale/2, scale, linesize, GxEPD_BLACK); // h x w
    display.drawLine(x+scale/2,y,x+scale,y+scale/2,GxEPD_BLACK);
    display.drawLine(x+scale/2-1,y,x+scale-1,y+scale/2,GxEPD_BLACK);
    display.drawLine(x+scale/2,y+scale+1,x+scale,y+scale/2+1,GxEPD_BLACK);
    display.drawLine(x+scale/2-1,y+scale+1,x+scale-1,y+scale/2+1,GxEPD_BLACK);
  }
  else if (dir== 1){ // Rising
    display.fillRect(x+scale/2, y, scale/2+linesize, linesize, GxEPD_BLACK); // h x w
    display.fillRect(x+scale, y, linesize, scale/2+linesize, GxEPD_BLACK); // h x w
    display.drawLine(x,y+scale,x+scale,y,GxEPD_BLACK);
    display.drawLine(x+1,y+scale,x+scale+1,y,GxEPD_BLACK);
  }  
}

void addcloud(int x, int y, int scale) {
  #define linesize 3
  display.fillCircle(x-scale/2, y-scale/2, scale, GxEPD_BLACK);                     // Left most circle - outer
  display.fillCircle(x-scale/2, y-scale/2, scale-linesize, GxEPD_WHITE);            // Left most circle - inner
  display.fillRect(x-scale/2, y+scale/2-2, scale*4.4, linesize,GxEPD_BLACK); // Lower line
  display.fillCircle(x+scale*3.8, y-scale/2, scale, GxEPD_BLACK);                   // Right most circle - outer
  display.fillCircle(x+scale*3.8, y-scale/2, scale-linesize, GxEPD_WHITE);          // Right most circle - inner
  display.fillCircle(x+scale*2.8, y-scale*2, scale*1.3, GxEPD_BLACK);               // Larger upper-circle - outer
  display.fillCircle(x+scale, y-scale*1.6, scale, GxEPD_BLACK);                     // Left upper circle - outer
  display.fillCircle(x+scale, y-scale*1.6, scale-linesize, GxEPD_WHITE);            // Left upper circle - inner
  display.fillCircle(x+scale*2.8, y-scale*2, scale*1.3-linesize, GxEPD_WHITE);      // Larger upper circle - inner
  display.fillRect(x-scale/2, y-scale, scale*4.4, scale*1.37,GxEPD_WHITE);               // Clear inside the cloud
  display.fillRect(x-scale/2, y+scale/2-linesize*1.5, scale*4.4, linesize,GxEPD_WHITE);  // Clear inside the cloud
  display.fillRect(x+scale, y-scale*2, scale*2.8, scale*2,GxEPD_WHITE);                  // Clear inside the cloud
}

void addrain(int x, int y, int scale){
  x = x + scale;
  for (int i = 0; i < 4; i++){
    display.drawLine(i*scale+x-scale*0.9,y+scale,i*scale+x-scale*0.9*1.5,y+scale*2,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9+1,y+scale,i*scale+x-scale*0.9*1.5+1,y+scale*2,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9+2,y+scale,i*scale+x-scale*0.9*1.5+2,y+scale*2,GxEPD_BLACK);
  }
}

void addsnow(int x, int y, int scale){
  int dxo, dyo, dxi, dyi;
  x = x - scale / 1.2;
  y = y + scale * 1.2;
  for (int flakes = 0; flakes < 4;flakes++){
    for (int i = 0; i <360; i = i + 45) {
      dxo = 0.5*scale * cos((i-90)*3.14/180); dxi = dxo*0.1;
      dyo = 0.5*scale * sin((i-90)*3.14/180); dyi = dyo*0.1;
      display.drawLine(dxo+x+0+flakes*1.5*scale,dyo+y,dxi+x+0+flakes*1.5*scale,dyi+y,GxEPD_BLACK); 
    }
  }
}

void addtstorm(int x, int y, int scale){
  x = x + scale;
  y = y - scale / 4;
  for (int i = 0; i < 4; i++){
    display.drawLine(i*scale+x-scale*0.9,     y+scale,       i*scale+x-scale*0.9*1.5,   y+scale*1.8,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9+1,   y+scale,       i*scale+x-scale*0.9*1.5+1, y+scale*1.8,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9*1.5, y+scale*1.8,   i*scale+x-scale*0.9,       y+scale*1.8,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9*1.5, y+scale*1.8+1, i*scale+x-scale*0.9,       y+scale*1.8+1,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9,     y+scale*1.8,   i*scale+x-scale*0.8*1.5,   y+scale*2.4,GxEPD_BLACK);
    display.drawLine(i*scale+x-scale*0.9+1,   y+scale*1.8,   i*scale+x-scale*0.8*1.5+1, y+scale*2.4,GxEPD_BLACK);
  }
}

void addsun(int x, int y, int scale) {
  #define linesize 3
  int dxo, dyo, dxi, dyi;
  display.fillCircle(x, y, scale,GxEPD_BLACK);
  display.fillCircle(x, y, scale-linesize,GxEPD_WHITE);
  for (float i = 0; i <360; i = i + 45) {
    dxo = 2.2*scale * cos((i-90)*3.14/180); dxi = dxo * 0.65;
    dyo = 2.2*scale * sin((i-90)*3.14/180); dyi = dyo * 0.65;
    if (i == 0   || i == 180) {
      display.drawLine(dxo+x-1,dyo+y,dxi+x-1,dyi+y,GxEPD_BLACK);
      display.drawLine(dxo+x+0,dyo+y,dxi+x+0,dyi+y,GxEPD_BLACK); 
      display.drawLine(dxo+x+1,dyo+y,dxi+x+1,dyi+y,GxEPD_BLACK);
    }
    if (i == 90  || i == 270) {
      display.drawLine(dxo+x,dyo+y-1,dxi+x,dyi+y-1,GxEPD_BLACK);
      display.drawLine(dxo+x,dyo+y+0,dxi+x,dyi+y+0,GxEPD_BLACK); 
      display.drawLine(dxo+x,dyo+y+1,dxi+x,dyi+y+1,GxEPD_BLACK); 
    }
    if (i == 45  || i == 135 || i == 225 || i == 315) {
      display.drawLine(dxo+x-1,dyo+y,dxi+x-1,dyi+y,GxEPD_BLACK);
      display.drawLine(dxo+x+0,dyo+y,dxi+x+0,dyi+y,GxEPD_BLACK); 
      display.drawLine(dxo+x+1,dyo+y,dxi+x+1,dyi+y,GxEPD_BLACK); 
    }
  }
}

void MostlyCloudy(int x, int y, int scale){ 
  addsun(x+scale/3,y-scale*2.1,scale); 
  addcloud(x,y,scale); 
}

void MostlySunny(int x, int y, int scale){ 
  addcloud(x,y,scale); 
  addsun(x+scale/3,y-scale*2.1,scale); 
}
 
void Rain(int x, int y, int scale){ 
  addcloud(x,y,scale); 
  addrain(x,y,scale); 
} 

void Cloudy(int x, int y, int scale){
  addcloud(x,y,scale);
}

void Sunny(int x, int y, int scale){
  addsun(x,y,scale);
}

void ExpectRain(int x, int y, int scale){
  addsun(x+scale/3,y-scale*2.1,scale); 
  addcloud(x,y,scale);
  addrain(x,y,scale);
}

void Tstorms(int x, int y, int scale){
  addcloud(x,y,scale);
  addtstorm(x,y,scale);
}

void Snow(int x, int y, int scale){
  addcloud(x,y,scale);
  addsnow(x,y,scale);
}

void Nodata(int x, int y, int scale){
  if (scale == 10) display.setTextSize(3); else display.setTextSize(1);
  display.setCursor(x,y);
  display.println("?");
  display.setTextSize(1);
}

