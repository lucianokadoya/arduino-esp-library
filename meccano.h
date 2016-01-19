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

// list of Ports of Meccano Mini Board and GPIOs
const int PORT1 = 14
const int PORT2 = 13
const int PORT3 = 2
const int PORT4 = 12

// ==================================================
String START_OF_OPERATION = "0";
String DEVICE_GROUP = "0";
String HOST = "";
String PORT = "0";
String SSID_ID = "";
String SSID_PW = "";
int    LED = 0;

String HOST;
String PORT;
String DEVICE_GROUP;

// Other constants
const boolean debug = true;
#define BLOCK_SIZE 15

// Led status list
int STATUS_NO_CONNECTION[10] = {0,0,0,0,0,1,1,1,1,0};
int STATUS_DATA_SENT[10]     = {1,0,1,0,1,0,0,0,0,0};
int STATUS_DATA_ERROR[10]    = {1,1,1,1,1,1,1,1,1,0};

// Checkpoint in time
unsigned long CHECK_POINT = 0;

class meccano {

  public:
    meccano();
    ~meccano();
    // Setup functions
    void wifi_setup(String ssid, String password);
    void server_setup(String host, int port);
    void led_setup(int gpio);
    void clock_setup();
    // Registration functions
    String register(String mac);
    // Fact Functions
    String fact_create(String channel, String sensor, String value);
    boolean fact_send(String fact);
    // Data functions
    boolean data_exists();
    boolean data_sync();
    void data_show();
    boolean data_write(String fact);
    // Led functions
    void led_status(int status[]);
    // Message functions
    void messages_process(unsigned long elapsed_time);
    // Utility functions
    String getMacAddress();
    void checkpoint();
    boolean elapsed(int seconds);

  private:
    String registration_create(String mac);
    File data_open();
    String messages_get(String comando);
    void messages_execute();
}

#endif
