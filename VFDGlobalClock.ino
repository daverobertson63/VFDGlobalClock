#include <PinButton.h>

/**
  VFD Clock Code for PICO Wifi version
  Written October 2022 by Brian DiDonna. 
  Updated for Global clock with timeapi.io Dave Robertson 2022
  Created on: 02.12.22

*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <limits.h>
#include "secrets.h"


/*
#ifndef STASSID
#define STASSID "*****************"
#define STAPSK "*****************"
#endif
*/

#define GLOBALTIMEDELAY 3600000

const char *ssid = STASSID;
const char *pass = STAPSK;

WiFiMulti WiFiMulti;

// Use global settings if we get them from WIFI
long GlobalMinutes=-1;
long GlobalHours=-1;
long GlobalSeconds=-1;
boolean UseGlobalClock=false;
int wifiTries=0;
DynamicJsonDocument doc(256);





#define SO1 0
#define SO2 1
#define SO4 2
#define SO8 3

#define ST1 4
#define ST2 5
#define ST4 6

#define MO1 7
#define MO2 8
#define MO4 9
#define MO8 10

#define MT1 11
#define MT2 12
#define MT4 13

#define HO1 15
#define HO2 17
#define HO4 18
#define HO8 19

#define HT1 14
#define HT2 16

#define HrsSet 28
#define MinSet 27
#define SecSet 26

#define LED LED_BUILTIN
#define Sqw 21

// Add this button library to detect extract clicks
PinButton myButton(SecSet);

static const int kMinutesInDay = 24 * 60;


// Get the time delta in milliseconds between first and second, accounting for a possible
// integer overflow of the millis() function (this happens every 50 days or so)
// NOTE this will ALWAYS return a positive number
unsigned long timeDelta( unsigned long first, unsigned long second )
{
  if ( second >=  first ) {
    return second - first;
  } else {
    return ( ULONG_MAX - first ) + second + 1;
  }
}


void setup() {

  Serial.begin(9600);
  

  UseGlobalClock=false;

  pinMode (LED, OUTPUT);
  pinMode (Sqw, INPUT);

  pinMode (SO1, OUTPUT);
  pinMode (SO2, OUTPUT);
  pinMode (SO4, OUTPUT);
  pinMode (SO8, OUTPUT);
  pinMode (ST1, OUTPUT);
  pinMode (ST2, OUTPUT);
  pinMode (ST4, OUTPUT);
  
  pinMode (MO1, OUTPUT);
  pinMode (MO2, OUTPUT);
  pinMode (MO4, OUTPUT);
  pinMode (MO8, OUTPUT);
  pinMode (MT1, OUTPUT);
  pinMode (MT2, OUTPUT);
  pinMode (MT4, OUTPUT);
  pinMode (HO1, OUTPUT);
  pinMode (HO2, OUTPUT);
  pinMode (HO4, OUTPUT);
  pinMode (HO8, OUTPUT);
  pinMode (HT1, OUTPUT);
  pinMode (HT2, OUTPUT);
  pinMode (HrsSet, INPUT);
  pinMode (MinSet, INPUT);
  pinMode (SecSet, INPUT);

  digitalWrite (HrsSet, 0);
  digitalWrite (MinSet, 0);
  digitalWrite (SecSet, 0);


  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  Serial.println("Connecting to Wifi...");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pass);
}

const char *jigsaw_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFKTCCBM+gAwIBAgIQAbTKhAICxb7iDJbE6qU/NzAKBggqhkjOPQQDAjBKMQsw
CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX
Q2xvdWRmbGFyZSBJbmMgRUNDIENBLTMwHhcNMjIwMzE3MDAwMDAwWhcNMjMwMzE2
MjM1OTU5WjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEWMBQG
A1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEe
MBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMFkwEwYHKoZIzj0CAQYIKoZI
zj0DAQcDQgAEYnkGDyrIltjRnxoVdy/xgndo+WGMOASzs2hHeCjbJ1KplKJc/ciK
XCWq/4+pTzSiVgTFhRmCdLcU1Fa05YFNQaOCA2owggNmMB8GA1UdIwQYMBaAFKXO
N+rrsHUOlGeItEX62SQQh5YfMB0GA1UdDgQWBBRIzOWGCDBB/PMrMucSrjIKqlgE
uDAvBgNVHREEKDAmghVzbmkuY2xvdWRmbGFyZXNzbC5jb22CDWppZ3Nhdy53My5v
cmcwDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcD
AjB7BgNVHR8EdDByMDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vQ2xv
dWRmbGFyZUluY0VDQ0NBLTMuY3JsMDegNaAzhjFodHRwOi8vY3JsNC5kaWdpY2Vy
dC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMD4GA1UdIAQ3MDUwMwYGZ4EM
AQICMCkwJwYIKwYBBQUHAgEWG2h0dHA6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzB2
BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0
LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0Ns
b3VkZmxhcmVJbmNFQ0NDQS0zLmNydDAMBgNVHRMBAf8EAjAAMIIBfwYKKwYBBAHW
eQIEAgSCAW8EggFrAWkAdQDoPtDaPvUGNTLnVyi8iWvJA9PL0RFr7Otp4Xd9bQa9
bgAAAX+aFPh6AAAEAwBGMEQCICivjuh2ywUYvVpTKHo65JEheR8dFq8QvBgEiXfw
m6q6AiAkxAgz77oboGQGetNmab45+peY+nAGOfyW9vi9S1gMaAB3ADXPGRu/sWxX
vw+tTG1Cy7u2JyAmUeo/4SrvqAPDO9ZMAAABf5oU+GEAAAQDAEgwRgIhANKeTNMy
GqUsCo7ph7YMWzrhMuDeyP8xPSiCtFzKcn/eAiEAyv5lgCUQ6K14V13zYfL99wZD
LFcIP/KZ1y7nuPAksTAAdwCzc3cH4YRQ+GOG1gWp3BEJSnktsWcMC4fc8AMOeTal
mgAAAX+aFPiWAAAEAwBIMEYCIQD6535jWw776D4vjyupP2fBw26CBMpVT5++k4rR
xqeOXwIhAIbEaEKkEq6JtpWWfVpTyDkMpMfTuiqYVe6REy2XsmEhMAoGCCqGSM49
BAMCA0gAMEUCIH3r/puXZcX1bfUoBq2njuHe0bxWtvzDaz5k6WLYrazTAiEA+ePL
N6K5xrmaof185pVCxACPLc/BoKyUwMeC8iXCm00=
-----END CERTIFICATE-----
)EOF";

static int cnt = 0;

void loop() {

 
  static int timeInMinutes = 0;
  static int timeSeconds = 0;
  static int lastFlash = 0;
  static bool squareWaveLastState = false;
  static bool minReadLastState = false;
  static bool hourReadLastState = false;
  static bool secondReadLastState = false;

  static unsigned long lastMinutePress = 0;
  static unsigned long lastHourPress = 0;
  static unsigned long lastGlobalUpdate = 0;
   

  myButton.update();

  


//Serial.printf("Minutes Value %d\n",timeInMinutes);
  //Flash the LED on and off as the seconds advance.
  int sqwRead = digitalRead(Sqw);
  digitalWrite(LED, lastFlash == 0 ? 1 : 0 );
  if ( sqwRead && !squareWaveLastState ) {
    ++lastFlash;
    if ( lastFlash >= 2 ) {
      ++timeSeconds;
      lastFlash = 0;
    }
  }
  squareWaveLastState = sqwRead;

  // time setting code
  int minRead = digitalRead(MinSet);
  // Serial.printf("Minutes Value %d\n",MinSet);
  if (minRead && ( !minReadLastState || timeDelta( lastMinutePress, millis() ) > 500 ) ) {
    lastMinutePress = millis();
    timeInMinutes++;
    if ( timeInMinutes % 60 == 0 ) {
      timeInMinutes -= 60;
    }
    Serial.printf("Minutes Set %d\n",timeInMinutes);
  }
  minReadLastState = minRead;

  int hourRead = digitalRead(HrsSet);
  if (hourRead && ( !hourReadLastState || timeDelta( lastHourPress, millis() ) > 500 ) ) {
    lastHourPress = millis();
    timeInMinutes += 60;
  }
  hourReadLastState = hourRead;

  
  //int secondRead = digitalRead(SecSet);
  if (myButton.isSingleClick()) {
    timeSeconds = 0;
    UseGlobalClock = false;
    Serial.printf("Reset the seconds\n");
  }
   if (myButton.isDoubleClick()) {
     // This is gonna force a connection
     lastGlobalUpdate=0;
     wifiTries=0;
     UseGlobalClock = true;
    Serial.printf("Request Global Clock %d\n",wifiTries);

   }

  //if (secondRead && !secondReadLastState ) {
  //  timeSeconds = 0;
 // }
  secondReadLastState = hourRead;

  // rectify time state
  while ( timeSeconds >= 60 ) {
    timeSeconds -= 60;
    ++timeInMinutes;
  }

  while ( timeInMinutes >= kMinutesInDay ) {
    timeInMinutes -= kMinutesInDay;
  }

  // light it up!
  int secondOnes = timeSeconds % 10;
  digitalWrite (SO1, secondOnes & 1);
  digitalWrite (SO2, secondOnes & 2);
  digitalWrite (SO4, secondOnes & 4);
  digitalWrite (SO8, secondOnes & 8);

  int secondTens = timeSeconds / 10;
  digitalWrite (ST1, secondTens & 1);
  digitalWrite (ST2, secondTens & 2);
  digitalWrite (ST4, secondTens & 4);

  int minutesOfHour = timeInMinutes % 60;
  int minuteOnes = minutesOfHour % 10;
  digitalWrite (MO1, minuteOnes & 1);
  digitalWrite (MO2, minuteOnes & 2);
  digitalWrite (MO4, minuteOnes & 4);
  digitalWrite (MO8, minuteOnes & 8);

  int minuteTens = minutesOfHour / 10;
  digitalWrite (MT1, minuteTens & 1);
  digitalWrite (MT2, minuteTens & 2);
  digitalWrite (MT4, minuteTens & 4);

  int hours = timeInMinutes / 60;
  int hourOnes = hours % 10;
  digitalWrite (HO1, hourOnes & 1);
  digitalWrite (HO2, hourOnes & 2);
  digitalWrite (HO4, hourOnes & 4);
  digitalWrite (HO8, hourOnes & 8);

  

  int hoursTens = hours / 10;
  digitalWrite (HT1, hoursTens & 1);
  digitalWrite (HT2, hoursTens & 2);

  if ( UseGlobalClock)
  {
    Serial.println("Calling the global update from the internet");
    lastGlobalUpdate=millis();
    UseGlobalClock = GlobalTime();
    // If true we have successful time 
    if ( UseGlobalClock ) {
      Serial.println("Setting clock to Global clock time..");
      timeInMinutes = GlobalHours*60 + GlobalMinutes;
      timeSeconds=GlobalSeconds;        
      UseGlobalClock=false;
     
    }
    else {
      Serial.println("Failed to get the time from the internet..");
    }


  }

  //delay(30000);
}

boolean GlobalTime()
{
  boolean setGlobalState=false;
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    HTTPClient https;
    switch (cnt) {
      case 0:
        Serial.println("[HTTPS] using insecure SSL, not validating certificate");
        https.setInsecure(); // Note this is unsafe against MITM attacks
        cnt++;
        break;
      case 1:
        Serial.println("[HTTPS] using secure SSL, validating certificate");
        https.setCACert(jigsaw_cert);
        cnt++;
        break;
      default:
        Serial.println("[HTTPS] not setting any SSL verification settings, will fail");
        cnt = 0;
    }

    Serial.print("[HTTPS] begin...\n");
    https.useHTTP10(true);
    //if (https.begin("localhost:")) {  // HTTPS
    if (https.begin("https://www.timeapi.io/api/Time/current/zone?timeZone=Europe/Amsterdam")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          //String payload = https.getString();

          //Serial.println(payload);

          deserializeJson(doc, https.getStream());

          //Serial.println(doc.getString());
          // Parse response

          
          GlobalHours=doc["hour"];
          GlobalMinutes=doc["minute"];
          GlobalSeconds=doc["seconds"];
        
          setGlobalState=true;

        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
     
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
     
    }
  }
  return setGlobalState;
}


