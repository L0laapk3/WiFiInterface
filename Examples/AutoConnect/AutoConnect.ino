#include <WiFiInterface.h>

#ifndef LED_BUILTIN
  #define LED_BUILTIN D4
#endif
#define LED_BUILTIN_ON LOW 
#define LED_BUILTIN_OFF HIGH


void setup() {
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF); 
  pinMode(LED_BUILTIN, OUTPUT);


  Serial.begin(115200);
  Serial.println();
  
  WiFiInterface.connect(" WiFiInterface Demo", [](){ digitalWrite(LED_BUILTIN, LED_BUILTIN_ON); }, [](){ digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF); });
  
  delay(5000);
  ESP.restart();
}

void loop() { }