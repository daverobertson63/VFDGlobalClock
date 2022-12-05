# VFDGlobalClock

 Sketch for Bolt Industries VFD Clock with Pico Wifi

 This is a simple update that allows the excellent Bolt Industries VFD clock to be on Global time... 

 https://www.boltind.com/vfd-clock-assembly/

 It adds a simple wifi connection assuming you use the very excellent British designed PICO Wifi or PICO W.

 The main difference is that the PICO W uses a different pin for the LED.  Thats all - everything else works ok for the existing instructions.

 The kit comes with a standard Pico ohne Wifi but I wanted it to sync with the world. 



# Dependencies / Libs

* Arduino IDE >= 1.5
* PinButton 
* HTTPClient 
* ArduinoJson

# Usage

Create a new sketch and create a secrets.h file with the following wifi creds.

```
#ifndef STASSID
#define STASSID "*******"
#define STAPSK "********"
#endif
```

Now just download

I have used the PinButton library so it can detect a double click on the seconds switch.  If you double click it will use the free timeapi.io REST api like this: 

https://www.timeapi.io/api/Time/current/zone?timeZone=Europe/Amsterdam

Which returns this:

```
{
  "year": 2022,
  "month": 12,
  "day": 3,
  "hour": 12,
  "minute": 31,
  "seconds": 20,
  "milliSeconds": 351,
  "dateTime": "2022-12-03T12:31:20.351351",
  "date": "12/03/2022",
  "time": "12:31",
  "timeZone": "Europe/Amsterdam",
  "dayOfWeek": "Saturday",
  "dstActive": false
}
```

It should keep good time on its own and once the wifi gets the global time it should be fairly accurate.  You can still adjust with the minutes if need be.

Obviosuly its all hard coded - but you could use the Arduino WiFiManager libs to maybe do something more configurable - but its my clock and once its done - it does its job and tells me the time in Kyiv time since that is where I got my VFD tubes from. 

This is just an idea so much more could be done with it.

<img src='https://github.com/daverobertson63/VFDGlobalClock/blob/main/IMG_5377.JPG'/>
