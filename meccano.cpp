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
#include <ESP8266WiFi.h>
#include <FS.h>

// ==================================================
String START_OF_OPERATION = "0";
String DEVICE_GROUP = "0";
String HOST = "";
String PORT = "0";
String SSID_ID = "";
String SSID_PW = "";
int    LED = 0;

// list of Ports of Meccano Mini Board and GPIOs
const int PORT1 = 14
const int PORT2 = 13
const int PORT3 = 2
const int PORT4 = 12

// Other constants
const boolean debug = true;
#define BLOCK_SIZE 15

// Led status list
int STATUS_NO_CONNECTION[10] = {0,0,0,0,0,1,1,1,1,0};
int STATUS_DATA_SENT[10]     = {1,0,1,0,1,0,0,0,0,0};
int STATUS_DATA_ERROR[10]    = {1,1,1,1,1,1,1,1,1,0};

// Constructor
meccano::meccano() {}

// Destructor
meccano::~meccano() {}

/**
** WIFI SETUP
**/
void meccano::wifi_setup(String ssid, String password) {
  SSID_ID = ssid;
  SSID_PW = password;
}

/**
** Server SETUP
**/
void meccano::server_setup(String host, String port) {
  HOST = host;
  PORT = port;
}

/**
** LED STATUS SETUP
**/
void meccano::led_setup(int gpio) {
  LED = gpio;
}

/**
**  Check if file exists
**/
boolean meccano::file_exists() {
  return SPIFFS.exists("/data.csv");
}

/**
* Send all data of file and then remove it
**/
boolean meccano::file_send() {
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
    File f = file_open();
    String bloco = "[";
    while(f.available()) {
      String line = f.readStringUntil('\n');
      bloco += line;
      numLinhas++;
      if(numLinhas > (BLOCK_SIZE - 1)) {
        bloco += "]";
        Serial.println(bloco);
        fact_send(bloco);
        bloco = "[";
        numLinhas = 0;
      } else {
          bloco += ",";
      }
    }
    // If there is any remaining data to send...
    if(numLinhas > 0) {
        bloco = bloco.substring(0, bloco.length() - 1);
        bloco = bloco + "]";
        Serial.println(bloco);
        fact_send(bloco);
    }
    f.close();
    Serial.println("Erasing local data...");
    SPIFFS.remove("/data.csv");
  return true;
}

/**
*  Open the local data file
*/
File meccano::file_open() {
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
void meccano::file_show() {
  Serial.println("Local data content: ");
  File f = file_open();
  while(f.available()) {
      String line = f.readStringUntil('\n');
      Serial.println(line);
  }
  f.close();
}

/**
*  Write fact to local data file
*/
boolean meccano::file_write(String fact) {
  Serial.println("Writing data to local file");
  File f = file_open();
  if(f) {
    f.print(fact);
    f.println();
    f.close();
    if(debug) file_show();
    return true;
  } else {
    return false;
  }
}

/**
*  Create the fact
*/
String meccano::fact_create(String channel, String sensor, String value) {
  String f = "";
  f += "{";
  f += "\"channel\":\"" + channel + "\",";
  f += "\"start\":" + START_OF_OPERATION + ",";
  f += "\"delta\":" + String(millis())  + ",";
  f += "\"device_group\":" + String(DEVICE_GROUP)  + ",";
  f += "\"device\": \"" + String(DEVICE_ID)  + "\",";
  f += "\"sensor\":" + String(sensor)  + ",";
  f += "\"data\":" + valor;
  f += "}";
  if(debug) Serial.println(f);
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
    return false;
  }
  String envelope = String("POST ") + "http://" + HOST + ":" + PORT + "/" + DEVICE_ID +  "/ HTTP/1.1\r\n" +
             "Accept: application/json\r\n" +
             "Host: " + HOST + "\r\n" +
             "Content-Type:application/json\r\n" +
             "User-Agent: Meccano/1.0\r\n" +
             "Content-Length: " + String(fact.length()) + "\r\n" +
             "\r\n" +
             fact +
             "\r\n";
  if(debug) client.print(envelope);
  delay(10);
  while(client.available()) {
    String line = client.readStringUntil('\r');
  }
  Serial.println();
  Serial.println("Closing connection.");
  led_status(STATUS_DATA_SENT);
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
String meccano::registration_send(String mac) {
  int linha = 0;
  WiFiClient client;
  if (!client.connect(HOST , PORT)) {
    Serial.println("Conexao falhou");
    led_status(STATUS_NO_CONNECTION);
  }
  String dadosJson = registration_create(mac);
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
  if(debug) client.print(envelope);
  delay(100);
  while(client.available()) {
    String line = client.readStringUntil('\r');
    linha++;
    if (linha == 11) {
     device_group = line.substring(1, 4);
     return device_group;
    }
  }
 Serial.println();
 Serial.println("Closing Connection.");
 led_status(STATUS_DATA_SENT);
}

/**
**  Show the status of device using a LED
**/
void meccano::led_status(int status[]){
  // If led is not configured, skip
  if(LED == 0) return;
  int passo;
  for (passo = 0; passo < 10; passo++) {
    digitalWrite(LED, status[passo]);
    delay (100);
  }
}

/**
**  get time from server
**/
String meccano::time_get() {
  int linha = 0;
  String horaServidor;
  Serial.println("Getting time from server... hora do servidor...");
  WiFiClient client;
  unsigned long hora;
  if (!client.connect(HOST, PORT)) {
    Serial.println("Falha na conexao...");
    return "\0";
  }
  client.print(String("GET ") + "http://" + HOST + ":" + PORT + "/" + MAC_char + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(5000);
  while(client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.println(line);
    linha++;
    if (linha == 11) {
     horaServidor = line.substring(1, 14);
     return horaServidor;
    }
  }
  Serial.println();
  Serial.println("Fechando conexao...");
  return "\0";
}

/**
** Parse messages
**/
String meccano::messages_get(String comando) {
  int tam = comando.length();
  String retorno = "";
  for(int i = 0; i < tam; i++) {
    if(comando[i] != '\n' && comando[i] != '\r') {
      retorno += comando[i];
    }
  }
  return retorno;
}

/**
** Load and execute commands
**/
void meccano::messages_execute() {
  String comando;
  Serial.println("Obtendo comandos do servidor...");
  WiFiClient client;
  if (!client.connect(HOST, PORT)) {
    Serial.println("No connection...");
    led_status(STATUS_NO_CONNECTION);
    return;
  }
  client.print(String("GET ") + "http://" + HOST + ":" + PORT + "/" + MAC_char + " HTTP/1.1\r\n" +
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
  }
  Serial.println();
  Serial.println("Closing connection...");
}
