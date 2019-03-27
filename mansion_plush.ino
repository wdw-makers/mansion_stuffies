/*
 *  Mansion Plush Sketch
 *
 *  
 *
 *  Created by WDW Makers
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *     
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#include "eyes.h"
#include "ravenscroft22pt.h"

const char* ssid = "<YOUR SSID>";
const char* password = "<YOUR PASSWORD>";

const char* host = "wdwntnowapi.azurewebsites.net";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "3A B0 B1 C2 7F 74 6F D9 0C 34 F0 D6 A9 60 CF 73 A4 22 9D E8";

#define PIN 1
const int numPixels = 39;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, PIN, NEO_GRB  + NEO_KHZ800);

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

unsigned long lastTimeCheck = 0;  // the last time the output pin was toggled
unsigned long timeCheckDelay = 60000;
boolean waitChanged = false;
int waitTime = 13;
const byte numChars = 6;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;

// Hue Change variables
double minHue = .33;
double maxHue = .66;
double hue = maxHue;
int hueDir = -1;
double hueRate = .0001;

// Glow animation variables
double bright = 1.0;
uint32_t color;
double brightness[numPixels];
int dir[numPixels];
double rate[numPixels];
double off[numPixels];

// Eyes animation variables
unsigned long eyesTimer = 0;
unsigned long eyesTimerStart = 0;
bool eyes = false;
int blinkCount = 2;
int blinkSide = 0;

/*****************************************************
 * 
 * Setup
 * 
 ****************************************************/
void setup() {
  // Setup pins for tx rx to display
  Wire.pins(0, 2);
  Wire.begin(0, 2);
  
  // Init the esp8266
  WiFi.mode(WIFI_STA);

  strip.begin();
  clearStrip();

  // setup OLED Screen
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    // Something is wrong with the display.  Turn neopixels red for a second.
    for(int i=0; i<numPixels; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
    strip.show();
    delay(1000);
  }

  display.display();
  delay(2000); // Display the WDW Makers logo for 2 min
 
  display.clearDisplay();

  // Init brightness and directions for the glowing effect
  for(int i=0; i<numPixels; i++) {
    if(i%2 == 0) {
      brightness[i] = 0;
      dir[i] = 1;
      off[i] = random(1,5);
    } else {
      brightness[i] = .5;
      dir[i] = -1;
    }
    rate[i] = random(2,10);
  }
}

/*****************************************************
 * 
 * Loop
 * 
 ****************************************************/
void loop() {
  // Check the server for a new time
  checkWaitTime();

  // Check to see if we should display the eyes
  checkEyes();

  // Check to see if we should play the lightning sequence
  checkLightning();

  // continue the glow effect
  glow(60, 1);
  
  if(hueDir == 1) {
    hue += hueRate;
  } else {
    hue -= hueRate;
  }
  if(hue >= maxHue) {
    hue = maxHue;
    hueDir = -1;
  }
  if(hue <= minHue) {
    hue = minHue;
    hueDir = 1;
  }
}

/*****************************************************
 * 
 * Check Eyes
 * 
 ****************************************************/
void checkEyes() {
  if(!eyes) {
    if(random(1, 800) == 17) {
      eyes = true;
      eyesTimer = 0;
      eyesTimerStart = millis();
      display.clearDisplay();
      blinkSide = random(0, 2);
      blinkCount = random(1, 4);
      drawEyes();
    }
  } else {
    drawEyes();
  }
}

/*****************************************************
 * 
 * Draw Eyes
 * 
 ****************************************************/
void drawEyes() {
  if(eyesTimer < 4000) {
    if(blinkSide == 0) drawRightEye(false);
    else drawLeftEye(false);
  } else if(eyesTimer < 5000) {
    if(blinkSide == 0) drawRightEye(true);
    else drawLeftEye(true);
  } else if(eyesTimer < 36000) {
    if(blinkSide == 0) drawRightEye(false);
    else drawLeftEye(false);
  } else if(eyesTimer >= 36000) {
    eyesTimer = 0;
    eyesTimerStart = millis();
    blinkCount--;
    blinkSide = random(0, 2);
    if(blinkCount > 0) return;
    eyes = false;
    drawWaitTime();
    display.display();
  }
  eyesTimer += millis() - eyesTimerStart; 
}

void drawRightEye(bool wink) {
  if(wink) {
    display.fillRect(90,18,small_eyes_width, small_eyes_height, BLACK);
    display.drawBitmap(90,18, small_eyes_closed_data, small_eyes_width, small_eyes_height, WHITE);
    display.display();
  } else {
    display.fillRect(90,18,small_eyes_width, small_eyes_height, BLACK);
    display.drawBitmap(42,3, large_eyes_data, large_eyes_width, large_eyes_height, WHITE);
    display.drawBitmap(10,18, small_eyes_data, small_eyes_width, small_eyes_height, WHITE);
    display.drawBitmap(90,18, small_eyes_data, small_eyes_width, small_eyes_height, WHITE);
    display.display();  
  }
}

void drawLeftEye(bool wink) {
  if(wink) {
    display.fillRect(10,18,small_eyes_width, small_eyes_height, BLACK);
    display.drawBitmap(10,18, small_eyes_closed_data, small_eyes_width, small_eyes_height, WHITE);
    display.display();
  } else {
    display.fillRect(10,18,small_eyes_width, small_eyes_height, BLACK);
    display.drawBitmap(42,3, large_eyes_data, large_eyes_width, large_eyes_height, WHITE);
    display.drawBitmap(10,18, small_eyes_data, small_eyes_width, small_eyes_height, WHITE);
    display.drawBitmap(90,18, small_eyes_data, small_eyes_width, small_eyes_height, WHITE);
    display.display();  
  }
}

/*****************************************************
 * 
 * Check for a wait time on the server
 * 
 ****************************************************/
void checkWaitTime() {
  if(lastTimeCheck != 0) {
    if(lastTimeCheck + timeCheckDelay > millis()) {
      return;
    }
  }
  lastTimeCheck = millis();
  
    WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    strip.setPixelColor(1, strip.Color(0, 0, 255));
    strip.show();
    delay(500);
  }

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  if (!client.connect(host, httpsPort)) {
    return;
  }
  String url = "/api/v2/mobile/attraction/80010208";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Accept: application/json\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  int newWaitTime;
  while(client.available()) {
    String line = client.readStringUntil('\n');
    int start = line.indexOf("waitTime");

    if (start > -1) {
      if( line.indexOf("Closed") > 0) {
        newWaitTime = 666;
        break;
      } else if( line.indexOf("Temporary") > 0) {
        newWaitTime = 999;
        break;
      } else {
        int startTime = line.indexOf(": ", start);
        int endQuote = line.indexOf("min", startTime);
        String waitTimeStr = line.substring(startTime + 2, endQuote-1);
        newWaitTime = waitTimeStr.toInt();
        break;
      }
    } 
  }
  if(newWaitTime != waitTime) {
    waitChanged = true;
    waitTime = newWaitTime;
  }
  client.stop();
}

/*****************************************************
 * 
 * Check if we should play the lightning sequence
 * 
 ****************************************************/
void checkLightning() {
  if(waitChanged && !eyes) {
    waitChanged = false;
    fadeToBlack();
    lightning();
    lightning();
    delay(2000);
    lightning();
    lightning();
    delay(2000);
    lightning();
    lightning();
    delay(2000);
    fadeToBlue();
    drawWaitTime();
  }
}

/*****************************************************
 * 
 * Lightning
 * 
 * Randomly show 2-5 flashes of light on 4 random leds
 * 
 ****************************************************/
void lightning() {
  int start = random(0, numPixels - 2);
  int flashes = random(2, 6);
  for(int j=0; j<flashes; j++) {
    for(int i=0; i<4; i++) {
      strip.setPixelColor(start+i, strip.Color(100, 100, 100));    
    }
    strip.show();
    delay(random(20, 100));
    for(int i=0; i<4; i++) {
      strip.setPixelColor(start+i, strip.Color(0, 0, 0));    
    }
    strip.show();
    delay(random(20, 100));
  }
  delay(random(20, 500)); // wait between flashes
}

/*****************************************************
 * 
 * Clear Strip
 * 
 ****************************************************/
void clearStrip() {
  for(int i=0; i<numPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

}

/*****************************************************
 * 
 * Draw Wait Time on OLED
 * 
 ****************************************************/
void drawWaitTime(void) {
  display.clearDisplay();
  display.setFont(&Ravenscroft22pt7b);
  display.setCursor(4, 30);
  display.cp437(true);         
  
  display.setTextColor(WHITE);

  if(waitTime == 666) {
    display.print("Closed");    
  } else if(waitTime == 999) {
    display.print("Closed");    
  } else {
    display.print(waitTime);
    display.print(" minutes");    
  }
  
  display.display();
}

/*****************************************************
 * 
 * FadeOut LCD
 * 
 ****************************************************/
void fadeout() {
    for (int dim=150; dim>=0; dim-=10) {
    display.ssd1306_command(0x81);
    display.ssd1306_command(dim); //max 157
    delay(50);
  }
  
  for (int dim2=34; dim2>=0; dim2-=17) {
    display.ssd1306_command(0xD9);
    display.ssd1306_command(dim2);  //max 34
    delay(100);
  }
}

/*****************************************************
 * 
 * FadeIn LCD
 * 
 ****************************************************/
void fadein() {
    for (int dim=0; dim<=160; dim+=10) {
    display.ssd1306_command(0x81);
    display.ssd1306_command(dim); //max 160
    delay(50);
  }
  
  for (int dim2=0; dim2<=34; dim2+=17) {
    display.ssd1306_command(0xD9);
    display.ssd1306_command(dim2);  //max 34
    delay(100);
  }
}

/*****************************************************
 * 
 * Glow
 * 
 ****************************************************/
void glow(int wait, double rateold) {
  for(int x = 0; x <strip.numPixels(); x++)
  {
    double delta = .1 / rate[x];
    if(off[x] > 0) {
      off[x] -= delta;
      if(off[x] < 0) {
        off[x] = 0;
      } else {
        continue;
      }
    }
   
    if(dir[x] == 1) brightness[x] += delta;
    else brightness[x] -= delta;
    if(brightness[x] > .5) {
      brightness[x] = .5;
      dir[x] = -dir[x];
      rate[x] = random(2,10);
    }
    if(brightness[x] < .00) {
      brightness[x] = .00;
      dir[x] = 1;
      rate[x] = random(2,10);
      off[x] = random(1,5);
    }
    
    color = hslToRgb(hue, 1, brightness[x]*bright); 
    uint8_t r = (color >> 16) & 255; // 255
    uint8_t g = (color >> 8) & 255; // 122
    uint8_t b = color & 255; // 15
 
    strip.setPixelColor(x,r,g, b);
  }
  strip.show();
  delay(wait);
}

/*****************************************************
 * 
 * Fade To Black
 * 
 ****************************************************/
void fadeToBlack() {
  boolean done = false;
  double fade = 20 / 1000.0;
  while(!done) {
    done = true;
    for(int x = 0; x <strip.numPixels(); x++)
    {
     brightness[x] -= fade;
     if(brightness[x] <= .00) {
        brightness[x] = .00;
        dir[x] = 1;
     } else {
       done = false;
     }
      
     color = hslToRgb(hue, 1, brightness[x]*bright); 
     uint8_t r = (color >> 16) & 255; // 255
     uint8_t g = (color >> 8) & 255; // 122
     uint8_t b = color & 255; // 15
    
    
    /*int r1 = r-flicker;
    int g1 = g-flicker;
    int b1 = b-flicker;
    if(g1<0) g1=0;
    if(r1<0) r1=0;
    if(b1<0) b1=0;
    */
    strip.setPixelColor(x,r,g, b);
    }
    strip.show();
    delay(40);
  }
}

/*****************************************************
 * 
 * Fade to Blue
 * 
 ****************************************************/
 void fadeToBlue() {
  for(int j = 0; j <40; j++) {
    glow(40, .5);
  }
}

/*****************************************************
 * 
 * HSL to RGB
 * 
 ****************************************************/
uint32_t hslToRgb(double h, double s, double l) {
    double r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3.0);
    }

    return strip.Color(r * 255,g * 255,b * 255);
}
 
double threeway_max(double a, double b, double c) {
    return max(a, max(b, c));
}

double threeway_min(double a, double b, double c) {
    return min(a, min(b, c));
}

double hue2rgb(double p, double q, double t) {
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/6.0) return p + (q - p) * 6 * t;
    if(t < 1/2.0) return q;
    if(t < 2/3.0) return p + (q - p) * (2/3.0 - t) * 6;
    return p;
}
