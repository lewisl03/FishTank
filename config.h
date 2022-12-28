const char* ssid = ""; //name of your router
const char* password = ""; //password for your router
int brightLight = 14; //light pin
int dimLight = 27; //dim light pin
int errorLED = 12;
double latitude = 51.5072; //your latitude 
double longitude = 0.1276; //your longitude this kinda matters for timezones and when the sunsets/rises. set to london 
int sunRiseDelay = 60; //in minutes
int sunSetDelay = 300; //in minutes
int updateTimezoneDelay = 300 * 1000;
unsigned long timerDelay = 300 * 1000; // how often you want to use their api
//google home stuff
const char* AIO_USERNAME = ""; //this stuff can be setup via https://iotdesignpro.com/projects/google-assistant-controlled-led-using-ESP32-and-adafruit-io
const char* AIO_KEY = "";
const char* AIO_SERVER = "io.adafruit.com";
int AIO_SERVERPORT = 1883;
