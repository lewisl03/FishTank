#include <WiFi.h>
#include <WiFiUdp.h>
#include <string>
#include <stdio.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <time.h>
#include "config.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "packetHandler.h"


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
bool debugBool = true;
bool isError;
bool isLight;
bool isLightBright;
bool flash = true;
bool lightOverride = false;
bool isDay;
bool isDayPre;

time_t currentTime;
time_t sunSet;
time_t sunRise;
int time_offset = 0;


long lastTime_UpdateSunRiseSet = 0;
long lastTime_TimeZoneUpdate = 0;

char apiUrl[100];


const String webResponse = "{}";

HTTPClient http;


//google assistance
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe LED_Control = Adafruit_MQTT_Subscribe(&mqtt, "Lewisl03/feeds/Fishtank");
void MQTT_connect();


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //error led
  pinMode(errorLED, OUTPUT);

  //light
  pinMode(brightLight, OUTPUT);
  pinMode(dimLight, OUTPUT);
  //put on the dim light
  toggleDimLight();





  //connect to wifi

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Scanning for networks");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
    isError = true;
    return;
  }
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    isError = true;
  }
  Serial.println(WiFi.localIP());

  //register
  mqtt.subscribe(&LED_Control);





  //prepare url

  snprintf(apiUrl, 100, "https://api.sunrise-sunset.org/json?lat=%f&lng=%f&date=today&formatted=0", latitude, longitude);
  Serial.println(apiUrl);



  timeClient.begin();
  timeClient.setTimeOffset(time_offset);



  

  //end setup
  isError = false;
  Serial.print("Setup complete ");
}



void loop() {



  debug("checking connection");
  //check to see if wifi is still connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected");
    isError = true;
  }

  //updates current time
  timeClient.update();




  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(0))) {
    if (subscription == &LED_Control) {
      Serial.print(F("Got: "));
      Serial.println((char *)LED_Control.lastread);
      if (!strcmp((char *)LED_Control.lastread, "ON")) {
        isLight = true;
        debug("Turning on fish tank light via google home");
        lightOverride = true;
        updateFishLight(true);
      } else {
        isLight = true;
        updateFishLight(true);
        debug("Turning off fish tank light via google home");
        lightOverride = false;
      }
    }
  }






  currentTime = timeClient.getEpochTime();
  //debug("Update time %d", currentTime);




  //debug("Check here");
  if (currentTime > (lastTime_UpdateSunRiseSet + timerDelay) || lastTime_UpdateSunRiseSet == 0) {
    updateSunRiseSet();

    Serial.println("CT: ");
    Serial.println(currentTime);
    Serial.println("SS: ");
    Serial.println(sunSet);
    Serial.println("SR: ");
    Serial.println(sunRise);

    //to detect if we've had a change
    isDayPre = isDay;

    if (currentTime < (sunRise + (sunRiseDelay * 1000)) || currentTime > (sunSet + (sunSetDelay * 1000))) {
      //because its night we want to dim the light
      isLightBright = false;
      Serial.println("Night");
      isDay = false;
    } else {
      isLightBright = true;
      isDay = true;
      Serial.println("Day");
    }

    if (isDayPre != isDay) {
      //the day has changed so update it
      updateFishLight(true);
    }
    //update when it was ran
    lastTime_UpdateSunRiseSet = currentTime;
  }










  checkError();

  //debug("End loop");
  delay(1000);
  //end
}



bool updateFishLight(bool override) {
  debug("Turning led %d isbright? %d", isLight, isLightBright);

  // if (!isLight && !override){ return false;}
  debug("Passed");


  if (isLightBright) {
    toggleBrightLight();

  } else {
    toggleDimLight();
  }
  debug("Done with dat");
  return true;
}


void toggleBrightLight() {
  digitalWrite(dimLight, LOW);
  digitalWrite(brightLight, HIGH);
  isLightBright = true;
  return;
}
void toggleDimLight() {
  digitalWrite(brightLight, LOW);
  digitalWrite(dimLight, HIGH);
  isLightBright = false;
  return;
}



void checkError() {
  digitalWrite(errorLED, (!flash || isError) ? HIGH : LOW);
  flash = (!flash || isError) ? true : false;
}



bool updateSunRiseSet() {

  HTTPClient http;


  http.begin(apiUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {

    String payload = http.getString();


    JSONVar myObject = JSON.parse(payload);
    JSONVar keys = myObject.keys();

    JSONVar value = myObject[keys[0]];



    const char *sunrise_str = value["sunrise"];

    struct tm time_info;


    strptime(sunrise_str, "%Y-%m-%dT%H:%M:%S+00:00", &time_info);
    sunRise = mktime(&time_info);



    if (currentTime > (lastTime_TimeZoneUpdate + updateTimezoneDelay || lastTime_TimeZoneUpdate == 0)) {

      Serial.println("Checking timezone");
      //update timezone
      int time_offset_hours, time_offset_minutes;
      sscanf(sunrise_str, "%*[^+]%d:%d", &time_offset_hours, &time_offset_minutes);
      long new_time_offset = (time_offset_hours * (60 * 60)) + (time_offset_minutes * 60);


      if (time_offset != new_time_offset) {

        Serial.print("Updated timezone to");
        Serial.println(new_time_offset);
        Serial.print("Was: ");
        Serial.println(time_offset);

        time_offset = new_time_offset;
        timeClient.setTimeOffset(time_offset);
      }
      //update when it was ran
      lastTime_TimeZoneUpdate = currentTime;
    }


    debug("Updated sunrise: &s", sunrise_str);

    //update sunset
    const char *sunset_str = value["sunset"];
    strptime(sunset_str, "%Y-%m-%dT%H:%M:%S+00:00", &time_info);
    sunSet = mktime(&time_info);

    //update sunrise
    debug("Updated sunset: &s", sunset_str);

  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    isError = true;
    return false;
  }

  return true;
}





void MQTT_connect() {
  int ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  int retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}




void debug(const char *format, ...) {
  if (!debugBool) { return; }

  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.println(buffer);
}
