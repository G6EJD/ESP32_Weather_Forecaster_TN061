# ESP32_Weather_Forecaster_TN061
A weather forecaster based on the Zambretti algorithm using a 2.9" e-paper display

BETA software:

Install the GxEPD Waveshare driver library from Github

Edit the credentials file for your own WiFi

Connect a BMP180 or )*% to pins 13,15, Vcc and Gnd


It's an SPI bus connection, so connect a display to the ESP32 like this:

LOLIN32 LITE/Other ESP32
CS    = 5;
MOSI  = 23 // also known as DIN
CLK   = 18
CS    = 5
DC    = 17
RST   = 16
BUSY  = 4

