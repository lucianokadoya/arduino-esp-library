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

class meccano {
  public:
    meccano();
    ~meccano();
    void wifi_setup(String ssid, String password);
    void server_setup(String host, String port);
    void led_setup(int gpio);
    boolean file_exists();
    boolean file_send();
    String fact_create(String channel, String sensor, String value);
    boolean fact_send(String fact);
    String registration_create(String mac);
    void led_status(int status[]);
    File file_open();
    void file_show();
    boolean file_write(String fact);
    String registration_send(String mac);
    String time_get();
    String messages_get(String comando);
    void messages_execute();
}

#endif
