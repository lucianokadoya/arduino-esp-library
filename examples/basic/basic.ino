#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "meccano.h"

int previous = HIGH;

const char* version = "0.0.1";

meccano m;

void setup() {
  Serial.begin(9600);
  m.led_setup(DPORT1);
  m.setup("*******", "*******", "server", 80, version);
  Serial.println("Setting ports...");
  pinMode(DPORT2, INPUT);
  Serial.println("ready!");
}

void loop() {
  int button = digitalRead(DPORT2);
  if(button == LOW && button != previous) {
    Serial.println("Button pressed!");
    // Create a fact representing that button has been pressed.
    // The channel will be named "switch".
    // We are choosing a sensor number 1.
    // The value 1 represents the button press.
    String fact = m.fact_create("switch", 1, 1);
    // Send the fact to the server
    m.fact_send(fact, MODE_PERSISTENT);
    delay(1000);
  }
  previous = button;
  // Read and process messages from server every 1m
  m.messages_process(60000);
}
