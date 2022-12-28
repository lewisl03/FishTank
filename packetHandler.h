WiFiServer wifiServer(93);

void socketSetup() {
  wifiServer.begin();
}

void socketEndLoop() {}



void socketListenerLoop(void* parameter) {
  //WiFiClient client = wifiServer.available();
  Serial.println("Socket Thread: ");
  Serial.print(Serial.println(xPortGetCoreID()));
  socketSetup();



  for (;;) {

 //   if (client) {
   //   while (client.connected()) {
        // Poll for data
  //      while (client.available() > 0) {
    //      char c = client.read();
    //      Serial.write(c);
   //     }
   //   }
   // }
  }
}
