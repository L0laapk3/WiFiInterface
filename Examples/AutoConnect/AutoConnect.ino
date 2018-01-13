#include <WiFiInterface.h>

#ifndef LED_BUILTIN
	#define LED_BUILTIN D4
#endif
#define LED_BUILTIN_ON LOW 
#define LED_BUILTIN_OFF HIGH


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);


  Serial.begin(115200);
  Serial.println();
  
  //function that gets called when user attention is required, and when user succesfully logged in to new network.
  WiFiInterface.connect([](){ digitalWrite(LED_BUILTIN, LED_BUILTIN_ON); }, [](){ digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF); });
  
  delay(5000);
  ESP.restart();
}

void loop() { }