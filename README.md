# About Meccano IoT Project

Meccano project is a multi-purpose IoT (Internet of Things) board and software platform created by Luciano Kadoya, Rogério Biondi, Diego Osse and Talita Paschoini. Its development started in early 2014 as a closed R&D project in the Software Architecture Division, with the aim of creating a board which is robust, based on a modern microprocessor (ESP8266), cheap, easy to implement and deploy through the 750 retail stores to perform several functions, such as:

- Count the number of visitors in each store to calculate the sales/visits ratio;
- Get the vote/feedback of users regarding the services;
- Voice marketing;
- Energy saving initiatives;
- Beacons and interaction of the customer in the physical store;
- Several other undisclosed applications;

Different from other ESP8266 projects, Meccano board has been heavily tested in retail stores and adjusted to be safe against RF (radio frequency) interferences. The physical store is an inhospitable environment since there are several hundreds of electronic products, such as TVs, computers, sound and home theaters as well as electronic home appliances.

The project is still in its early stages and will evolve in the future. Magazine Luiza will plan the backlog and sponsor the project. It has been open-sourced because it´s the first initiative to create a board based on ESP8266 in Brazil and we are really excited with the possibilities. Magazine Luiza has a passion for innovations and contribution to the development of technology. So you are invited to join us because your support/collaboration is welcome!


# Meccano-Arduino-Library

Meccano arduino-esp-library is a client library for Meccano Mini Board, NodeMCU and ESP8266 Generic Boards (ESP-07 and ESP-12).

## Features:

 - Simple to use
 - Integration to Meccano Gateway    
    - Create and send facts
    - When no connection available, store facts in a local data file.
    - Check and execute messages from gateway
	- **OTA (On the Air) Updates (new!)**


## requirements

In order to use meccano-arduino-esp library you should setup the Arduino ESP8266.

https://github.com/esp8266/Arduino

You should use the stage version, 2.1.0-rc1 due to the file system feature.
http://arduino.esp8266.com/staging/package_esp8266com_index.json
Documentation: http://esp8266.github.io/Arduino/versions/2.1.0-rc1/


## Installation

1. Download the zip from GIT Hub to a local directory (e. g. Dowloads)
2. Rename file Downloads/arduino-esp-library-master.zip > meccano-esp-library.zip
2. Open your Arduino IDE
3. Select Sketch > Include Library > Add .ZIP Library...
4. Select the Downloads/meccano-esp-library.zip

### Mininum Meccano Program

You need to include the meccano library in your code:

```
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "meccano.h"

meccano m;

const char *version = "0.0.1";

void setup() {
  // Initialize the Serial Port
  Serial.begin(9600);
  // Setup the Meccano Library
  m.setup("my_ap", "my_pwd", "myserver.meccano.iot", 80, version);  
  // Ready to go...
}

void loop() {
  // Your code here
  // ...
  // The device will receive messages and announce every 1 minute.
  m.process_messages(60000);  
}
```

### Ports

```
const int PORT1 = 14
const int PORT2 = 13
const int PORT3 = 2
const int PORT4 = 12
```

In meccano board, PORT1 uses GPIO14, PORT2 uses GPIO13 and so on. You may use the constants in order to simplify the use of GPIO functions. For example:

```
void setup() {
  // Setup commands...
  // Port functions
  pinMode(PORT1, OUTPUT);
  digitalWrite(PORT1, HIGH);
  delay(2000);
  digitalWrite(PORT1, LOW);
}
```

### Functions

#### Setup functions ####


##### void led_setup(int gpio); #####

If you want to use the led feature, attach it to one of the digital ports (with a 330R+ resistor) and meccano library will use it for showing the status of its operations. When setting up, you must choose one of the available ports for operation (e. g. PORT3)

```
m.led_setup(PORT3);
```

##### void buzz_setup(int gpio); #####

If you want to use the buzz feature, attach it to one of the digital ports and you may use it according your need.
When setting up, you must choose one of the available ports for operation (e. g. PORT2)

```
m.buzz_setup(PORT2);
```


##### void setup(String ssid, String password, String server, int port, const char* version); #####

The setup() will do the following functions:

1. Setup and get the device id (mac-address);
2. Setup the wifi connection to the access point (AP). You must provide your AP credentials (ssid and password);
3. Setup the connection to gateway. You must pass the host (or IP) and port where the meccano gateway is running;
4. It will register the device in the Meccano Network;
5. It will get the clock information.

```
m.wifi_setup("My_SSID", "MY_SSID_PASSWORD", "meccano.meccano.iot", 80, "0.0.1");
```


#### Fact functions ####

Facts are the representation of a physical event. They are data captured by the sensors of Meccano Mini Board. It can be a temperature sensor, a line infrared or PIR sensor or others. The data of a fact is represented by a numeric value. Examples: for a temperature sensor it should be a number between -100 and 100 representing the celsius measure, or if you create a button it can be the number of times which it has been pressed by the user.


##### String fact_create(String channel, int sensor, int value); #####

When you create a fact, you must specify a channel. The channel is a class that identify which kind of information you want to send to the meccano gateway. Besides the channel, each fact must specify the sensor. You must define a number for each sensor connected to your meccano mini board. Let's consider, for example, that you have a PIR sensor connected in one port and a temperature sensor in other port. for identification, you should consider the PIR as sensor 1 and temperature as sensor 2. If you have several identical appliances which the same configuration, you must keep the same configuration of sensor for all devices. The value of the sensor is the data captured of them. This can be a value of temperature, a voltage or whatever you need to collect.


##### boolean fact_send(String fact, int mode); #####

Send a fact to the meccano gateway.

```
// Read the PIR sensor. If there is someone in the area, create a fact and send to the gateway
if(digitalRead(PORT1) == HIGH) {
  String fact = m.fact_create("PRESENCE", 1, 1)
  m.fact_send(fact, MODE_PERSISTENT);
}
```

**mode** refers to the mode of persistence. It may assume two values: MODE_PERSISTENT and MODE_NON_PERSISTENT. 

- **MODE_PERSISTENT**: if there is no wifi connection available, the data will be persisted to the local database. When there is another data sent to the gateway, if the connection is restablished, local stored data will be sent to the gateway automatically.
- **MODE_NON_PERSISTENT**: if there is no connection, data will be discarded.


#### Data functions ####



##### boolean data_exists(); #####

Returns true or false if there is local data stored on device.



##### boolean data_sync(); #####

Sends the data stored in the local device manually. The data_sync function is already automatically called in fact_send, but you may also force a synchronization if you need.

```
boolean exists = m.data_exists();
if(exists) {
  Serial.println("There is data stored in the device.");
  Serial.println("Sending to the gateway.");
  m.data_sync();
}

```


##### void data_show(); #####

Shows the data stored in the local device. This should be used for debug purpose only.

```
if( m.data_exists() ) m.data_show();
```



#### Led Functions ####

If you need, you may connect a led for getting status of communication and operation. Meccano library has a led functionality ready to use.



##### void led_status(String status);  #####

You may create a custom pattern for blinking the led connected to the meccano mini board.

```
String STATUS_FAIL = "1010101010";
m.led_status(STATUS_FAIL);
```

In the example above, if there is a fail, the led will blink 5 times. Each blink duration is 100ms.


#### Buzz Functions ####

If you need, you may connect a buzz for generating audible notifications. Meccano library has a buzz functionality ready to use.



##### void buzz(String status);  #####

You may create a custom pattern for buzzing .

```
String STATUS_OK = "1111100000";
m.buzz(STATUS_OK);
```

In the example above, if there is a fail, the buzz will sound 500ms. Each 1 or 0 duration 100ms.




##### Messages functions #####

The gateway may send some messages to the devices, such as a REBOOT, for restarting the device, a BLINK for blinking the led but also custom messages you may implement yourself. These messages can be switching on/off an actuator, getting a measure, reading the data of a sensor or any other



##### void messages_process(unsigned long elapsed_time); #####

messages_process executes the following functions:

1. Receive messages from Meccano Network/Gateway (such as BLINK and REBOOT) and executes them;
2. Announce device to the gateway;
3. Check if there is local data stored locally and fires up the synchronization to the gateway;

This function must be called in the loop function and you'll pass the number of miliseconds for verification. This should be 60.000 (1m or more, depending on your application)

```
void loop() {
  // Read and process messages from server every 1m
  m.messages_process(60000);
}
```

#### Utility functions ####

##### String get_id() #####

Gets the id (mac-address) of the device.

```
void setup() {  
  Serial.begin(9600);
  m.setup("*****", "*****", "meccano.server.iot", 80, "0.0.1");  
  Serial.println("Mac Address: " + m.get_id());
}
```
