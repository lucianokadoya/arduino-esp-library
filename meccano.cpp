/*
* Meccano IOT Gateway
*
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "FS.h"
#include "meccano.h"

String SSID_ID = "";
String SSID_PW = "";

char *HOST;
uint16_t PORT = 80;
String DEVICE_GROUP = "0";

int LED = 0;
int BUZZ = 0;

// Notifications
String STATUS_NO_CONNECTION  = "0000011110";
String STATUS_CONNECTION_ON  = "1111111111";
String STATUS_CONNECTION_OFF = "0000000000";
String STATUS_DATA_SENT      = "1010100000";
String STATUS_DATA_ERROR     = "1111111110";

int CONNECTION_RETRIES = 50;

// Timestamp for start of operation
String START_OF_OPERATION = "0";

// Array of checkpoints
unsigned long CHECK_POINT[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Mac address
String MAC_ADDRESS = "99:99:99:99:99:99";

// DEBUG. Default = ON
boolean DEBUG = false;

#define BLOCK_SIZE 15

// Constructor
meccano::meccano() {}

// Destructor
meccano::~meccano() {}


/**
**  All in one (main) setup function
**/
boolean meccano::setup(char *ssid, char *password, char *host, int port) {
  delay(5000);
  SPIFFS.begin();
  Serial.println();
  Serial.println("Meccano IoT");
  Serial.println("(c) 2015-2016 - Magazine Luiza");
  Serial.println("                Arquitetura de Solucoes");
  Serial.println();
  delay(3000);
  Serial.print("Free memory = ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Total flash memory = ");
  Serial.println(ESP.getFlashChipSize());
  delay(1000);
  boolean dev = device_setup();
  boolean wifi = wifi_setup(ssid, password);
  boolean server = server_setup(host, port);
  boolean reg = registration();
  boolean clock = clock_setup();
  // if device group not received, then restart.
  if (DEVICE_GROUP == "0") {
   Serial.println("Device Group Unknown. Restarting...");
   ESP.restart();
  }
  return (dev && wifi && server && reg && clock);
}

/**
** WIFI SETUP
**/
boolean meccano::wifi_setup(char *ssid, char *password) {
  SSID_ID = ssid;
  SSID_PW = password;
  WiFi.begin(ssid, password);
  int retries = CONNECTION_RETRIES;
  Serial.println("Starting wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    led_status(STATUS_CONNECTION_ON);
    Serial.print(".");
	led_status(STATUS_CONNECTION_OFF);
    retries--;
    if(retries <= 0) {
      Serial.println();
      ESP.restart();
    }
    delay(1000);
  }
  Serial.println();
  return true;
}

/**
** Server SETUP
**/
boolean meccano::server_setup(char *host, int port) {
  Serial.println("Configuring server...");
  HOST = host;
  PORT = port;
  return true;
}

/**
** LED STATUS SETUP
**/
boolean meccano::led_setup(int gpio) {
  LED = gpio;
  pinMode(LED, OUTPUT);
  return true;
}

/**
** BUZZ SETUP
**/
boolean meccano::buzz_setup(int gpio) {
  BUZZ = gpio;
  pinMode(BUZZ, OUTPUT);
  return true;
}

/**
**  Clock setup
**/
boolean meccano::clock_setup() {
  int lineNumber = 0;
  String serverTime;
  Serial.println("Getting time from server...");
  WiFiClient client;
  if (!client.connect(HOST, PORT)) {
    Serial.println("Connection Failed...");
    led_status(STATUS_NO_CONNECTION);
    ESP.restart();
  }
  client.print(String("GET ") + "http://" + HOST + ":" + PORT + "/" + MAC_ADDRESS + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(5000);
  while(client.available()) {
    String line = client.readStringUntil('\r');
    lineNumber++;
    if (lineNumber == 11) {
     serverTime = line.substring(1, 14);
     // Check if the timestamp
     String firstDigit = serverTime.substring(0, 1);
     if(!isDigit(firstDigit.charAt(0))) {
       Serial.println("Time not received or not authorized to connect to Meccano Network. Rebooting...");
       led_status(STATUS_NO_CONNECTION);
       ESP.restart();
     }
     START_OF_OPERATION = serverTime;
     Serial.println("Start of operation: " + START_OF_OPERATION);
     break;
    }
  }
  Serial.println("Closing Connection...");
  // Create the checkpoint 0 for message processing
  checkpoint(0);
  return true;
}

/**
**  Create the registration record
**/
String meccano::registration_create(String mac) {
  String reg = "";
  reg += "{";
  reg += "\"operation\": \"PUT\",";
  reg += "\"device\": \"" + mac + "\"";
  reg += "}";
  return reg;
}

/**
**  Register device in the Meccano Gateway
**/
boolean meccano::registration() {
  Serial.println("Starting Registration...");
  int lineNumber = 0;
  WiFiClient client;
  if (!client.connect(HOST , PORT)) {
    Serial.println("Connection failed.");
    led_status(STATUS_NO_CONNECTION);
    ESP.restart();
  }
  String dadosJson = registration_create(MAC_ADDRESS);
  String device_group;
  String envelope = String("PUT ") + "http://" + HOST + "/api/registration/" + " HTTP/1.1\r\n" +
             "Accept: application/json\r\n" +
             "Host: " + HOST + "\r\n" +
             "Content-Type: application/json\r\n" +
             "User-Agent: Meccano/1.0\r\n" +
             "Content-Length: " + String(dadosJson.length()) + "\r\n" +
             "\r\n" +
             dadosJson + "\r\n" +
             "\r\n";
  if(DEBUG) Serial.println(envelope);
  client.print(envelope);
  delay(500);
  String line = "";
  while(client.available()) {
    String line = client.readStringUntil('\r');
	if(DEBUG) Serial.println(line);
	int dgLen = line.length();
	DEVICE_GROUP = line.substring(1, dgLen);
 }
 Serial.println("Device Group: " + DEVICE_GROUP);
 Serial.println("Closing Connection.");
 led_status(STATUS_DATA_SENT);
 return true;
}


/**
**  Show the status of device using a LED
**/
void meccano::led_status(String status){
	notify(status, LED);
}

/**
**  Buzz notification
**/
void meccano::buzz(String status){
	notify(status, BUZZ);
}

/**
**  Buzz notification
**/
void meccano::notify(String status, int gpio){
  // If buzz is not configured, skip
  if(gpio == 0) return;
  int statusSize = status.length();
  int pass;
  for (pass = 0; pass < statusSize; pass++) {
	char charact = status[pass];
	if(charact == '1') {
		digitalWrite(gpio, HIGH);
	} else {
		digitalWrite(gpio, LOW);
	}
    delay (100);
  }
}

/**
**  Get the mac-address of the ESP
**/
boolean meccano::device_setup() {
  String octet;
  byte MAC_array[6];
  Serial.println("Getting the mac-address...");
  WiFi.macAddress(MAC_array);
  String mac = "";
  for(int i=0; i<6; i++) {
    octet = String(MAC_array[i], HEX);
    mac += String(octet);
    if(i < 5)  mac += ":";
  }
  MAC_ADDRESS = mac;
  Serial.println("Device ID: " + MAC_ADDRESS);
  return true;
}

/**
**  Set a checkpoint in time
**/
void meccano::checkpoint(int id) {
  CHECK_POINT[id] = millis();
  Serial.println("Checkpoint " + String(id) + " created at " + String(CHECK_POINT[id]));
}

/**
**  Check if has passed n-seconds
**/
boolean meccano::elapsed(int id, unsigned long elapsed_time) {
  unsigned long since_checkpoint = (millis() - CHECK_POINT[id]);
  return(since_checkpoint >= elapsed_time);
}

/**
** Parse messages
**/
String meccano::messages_get(String command) {
  int len = command.length();
  String ret = "";
  for(int i = 0; i < len; i++) {
    if(command[i] != '\n' && command[i] != '\r') {
      ret += command[i];
    }
  }
  return ret;
}

/**
** Load and execute commands
**/
void meccano::messages_execute() {
  String command;
  Serial.println("Get commands from server...");
  WiFiClient client;
  if (!client.connect(HOST, PORT)) {
      Serial.println("No connection...");
      led_status(STATUS_NO_CONNECTION);
      return;
  }
  client.print(String("GET ") + "http://" + HOST + ":" + PORT + "/" + MAC_ADDRESS + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500);
  while(client.available()) {
    String line = client.readStringUntil('\r');
    line = messages_get(line);
    if (line == "REBOOT") {
      Serial.println("REBOOT command received. Restarting device...");
      ESP.restart();
    }
    if (line == "BLINK") {
      Serial.println("BLINK command received...");
      for (int i = 0; i < 20 ; i++) led_status(STATUS_DATA_ERROR);
    }
	  if (line == "PURGE") {
	    Serial.println("PURGE command received...");
	    data_format();
	  }
	  if (line == "FORCE_SYNC") {
      Serial.println("FORCE_SYNC command received...");
      data_sync();
	  }
  }
  Serial.println();
  Serial.println("Closing connection...");
}

/**
**  Process messages
**/
void meccano::messages_process(unsigned long elapsed_time) {
  // Uses the checkpoint 0 for messages processing
  if(elapsed(0, elapsed_time)) {
    messages_execute();
    // Check if there is data in the local device to send
    if(data_exists()) {
      data_sync();
    }
    // Create a new checkpoint
    checkpoint(0);
  }
}

/**
*  Create the fact
*/
String meccano::fact_create(String channel, int sensor, int value) {
  String f = "";
  f += "{";
  f += "\"channel\":\"" + channel + "\",";
  f += "\"start\":" + START_OF_OPERATION + ",";
  f += "\"delta\":" + String(millis())  + ",";
  f += "\"device_group\": \"" + String(DEVICE_GROUP)  + "\",";
  f += "\"device\": \"" + String(MAC_ADDRESS)  + "\",";
  f += "\"sensor\":" + String(sensor)  + ",";
  f += "\"data\":" + String(value);
  f += "}";
  if(DEBUG) Serial.println(f);
  return f;
}

/**
**  Send fact
**/
boolean meccano::fact_send(String fact) {
  WiFiClient client;
  if (!client.connect(HOST , PORT)) {
    Serial.println("Connection lost.");
    led_status(STATUS_NO_CONNECTION);
    data_write(fact);
    return false;
  }
  fact = "[ " + fact + " ]";
  String envelope = String("POST ") + "http://" + HOST + ":" + PORT + "/" + MAC_ADDRESS +  "/ HTTP/1.1\r\n" +
             "Accept: application/json\r\n" +
             "Host: " + HOST + "\r\n" +
             "Content-Type:application/json\r\n" +
             "User-Agent: Meccano/1.0\r\n" +
             "Content-Length: " + String(fact.length()) + "\r\n" +
             "\r\n" +
             fact +
             "\r\n";
  if(DEBUG) Serial.println(envelope);
  client.print(envelope);
  delay(10);
  while(client.available()) {
    String line = client.readStringUntil('\r');
  }
  Serial.println("Data sent successfully to the Meccano Gateway.");
  Serial.println("Closing connection.");
  led_status(STATUS_DATA_SENT);
  return true;
}

/**
**  Check if file exists
**/
boolean meccano::data_exists() {
  return SPIFFS.exists("/data.csv");
}


/**
* Send all data of file and then remove it
**/
boolean meccano::data_sync() {
  Serial.println("Syncing data...");
  if(DEBUG) {
	  Serial.println("===");
	  data_show();
	  Serial.println("===");
  }
  Serial.println("Testing Connection...");
  WiFiClient client;
  if (!client.connect(HOST, PORT)) {
    Serial.println("No connection to server. The data will be sent in the future...");
    return false;
  }
  int numLinhas = 0;
  // If the file exists, send it
  Serial.println("Checking local data...");
  Serial.println("Local data exists. Sending...");
  File f = data_open();
  String block = "";
    while(f.available()) {
    String line = f.readStringUntil('\n');
    block += line;
    numLinhas++;
    if(numLinhas > (BLOCK_SIZE - 1)) {
	  Serial.println("++++");
      Serial.println(block);
	  Serial.println("++++");
      fact_send(block);
      numLinhas = 0;
    } else {
        block += ",";
    }
  }
  // If there is any remaining data to send...
  if(numLinhas > 0) {
      block = block.substring(0, block.length() - 1);
      Serial.println(block);
      fact_send(block);
  }
  f.close();
  Serial.println("Erasing local data...");
  SPIFFS.remove("/data.csv");
  return true;
}

/**
*  Open the local data file
*/
File meccano::data_open() {
  File f = SPIFFS.open("/data.csv", "a+");
  if(f) {
  } else {
    Serial.println("Could not open local data file...");
  }
  return f;
}

/**
*  Shows the content of the local data file
*/
void meccano::data_show() {
  Serial.println("Local data content: ");
  File f = data_open();
  while(f.available()) {
      String line = f.readStringUntil('\n');
      Serial.println(line);
  }
  f.close();
}

/**
*  Format the file system
*/
void meccano::data_format() {
  Serial.println("Formating the file system... ");
  SPIFFS.format();
}


/**
*  Write fact to local data file
*/
boolean meccano::data_write(String fact) {
  Serial.println("Writing to local database");
  File f = data_open();
  if(f) {
    f.print(fact);
    f.println();
    f.close();
    if(DEBUG) data_show();
    return true;
  } else {
    return false;
  }
}

/**
**  Get the ID (mac-address) of the device
**/
String meccano::get_id() {
  return MAC_ADDRESS;
}
