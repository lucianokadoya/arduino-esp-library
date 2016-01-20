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

class meccano {

  public:
    meccano();
    ~meccano();
    // Setup functions
    void wifi_setup(char *ssid, char *password);
    void server_setup(char *host, int port);
    void led_setup(int gpio);
    void clock_setup();
    void registration();
    // Led functions
    void led_status(int status[]);
    // Utility functions
    String getMacAddress();
    // Checkpoint functions
    void checkpoint(int id);
    boolean elapsed(int id, unsigned long elapsed_time);
    // Messages functions
    void messages_process(unsigned long elapsed_time);
    // Fact functions
    String fact_create(String channel, int sensor, int value);
    boolean fact_send(String fact);

  private:
    String messages_get(String command);
    void messages_execute();
    String registration_create(String mac);
    boolean data_exists();
    boolean data_send();
    File data_open();
    void data_show();
    boolean data_write(String fact);
};

#endif
