#include <WiFi.h>
#include <WiFiUdp.h>
#include <string>
#include <stdio.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <time.h>


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
bool debugBool = false;
bool isError = false;
bool flash = true;

const char* ssid = "EE-Hub-Q7Kr";
const char* password = "lunar-CAN-menu";
int light = 14;

time_t currentTime;
time_t sunSet;
time_t sunRise;

unsigned long timerDelay = 300 * 1000; //every 5 mins
unsigned long lastTime = 0;

double latitude = 51.441883;
double longitude = 0.370759;

char apiUrl[100];


const String webResponse = "{}";

HTTPClient http;


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
Serial.print("Connecting to ");
Serial.println(ssid);

//error led
pinMode(12, OUTPUT);
pinMode(14, OUTPUT);
digitalWrite(12, HIGH);




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




//prepare url

snprintf(apiUrl, 100, "https://api.sunrise-sunset.org/json?lat=%f&lng=%f&date=today&formatted=0", latitude, longitude);
Serial.println(apiUrl);



  timeClient.begin(); 
  timeClient.setTimeOffset(0);


  //end setup
isError = false;
Serial.print("Setup complete ");
}



void loop() {


debug("checking connection");
  //check to see if wifi is still connected
  if(WiFi.status()!= WL_CONNECTED){
     Serial.println("WiFi Disconnected");
     isError = true;
  }


    //updates current time
    timeClient.update();

  


  currentTime = timeClient.getEpochTime();




  debug("Update time &d",currentTime);

  


debug("Check here");




    if(currentTime > (lastTime + timerDelay) || lastTime == 0){
      updateSunRiseSet();

Serial.println("CT: ");
Serial.println(currentTime);
Serial.println("SS: ");
Serial.println(sunSet);
Serial.println("SR: ");
Serial.println(sunRise);




    if(currentTime < sunRise || currentTime > sunSet){
        digitalWrite(light, LOW);
    }else{
      digitalWrite(light, HIGH);
    }


      
    }






 
      












  if(isError){
    digitalWrite(12, HIGH);
  }else if(flash){
    digitalWrite(12, LOW);
    flash = !flash;


  }else{
    digitalWrite(12, HIGH);
    flash = !flash;
  }

    delay(1000);
}




bool updateSunRiseSet(){
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


    debug("Updated sunrise: &s", sunrise_str);

    //update sunset
   const char *sunset_str = value["sunset"];
    strptime(sunset_str, "%Y-%m-%dT%H:%M:%S+00:00", &time_info);
    sunSet = mktime(&time_info);

      //update sunrise
     debug("Updated sunset: &s", sunset_str);



   

  
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
         isError = true;
        return false;
      }

  lastTime = currentTime;
  return true;

}


void debug(const char *format, ...){
  if(!debugBool){return;}
  
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
  
}

