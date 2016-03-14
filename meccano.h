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
#ifndef MECCANO_H

#define MECCANO_H

#include <Arduino.h>
#include "FS.h"
#include <ESP8266WiFi.h>

// list of Ports of Meccano Mini Board and GPIOs
const int DPORT1 = 14;
const int DPORT2 = 13;
const int DPORT3 = 2;
const int DPORT4 = 12;

// Persistence Modes
const int MODE_PERSISTENT = 1;
const int MODE_NON_PERSISTENT = 0;

class meccano {


  public:
    meccano();
    ~meccano();
	
    // Setup functions
    boolean led_setup(int gpio);
	boolean buzz_setup(int gpio);
    boolean setup(char *ssid, char *password, char *host, int port);

    // Data functions
    boolean data_exists();
    boolean data_sync();
	void data_format();
    void data_show();

    // Led functions
    void led_status(String status);
	
	// Buzz functions
    void buzz(String status);
	
    // Checkpoint functions
    void checkpoint(int id);
    boolean elapsed(int id, unsigned long elapsed_time);

    // Messages functions
    void messages_process(unsigned long elapsed_time);

    // Fact functions
    String fact_create(String channel, int sensor, int value);
    boolean fact_send(String fact, int mode);

    // Utility functions
    String get_id();

  private:
    boolean device_setup();
    boolean wifi_setup(char *ssid, char *password);
    boolean server_setup(char *host, int port);
    boolean clock_setup();
    boolean registration();
    String messages_get(String command);
    void messages_execute();
    String registration_create(String mac);
    File data_open();
    boolean data_write(String fact);
	void notify(String status, int gpio);
};

#endif
