/*
    This file is part of wimaxcmgui.

    Copyright (c) MD: Minhazul Haq Shawon, 2012

    wimaxcmgui is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    wimaxcmgui is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with wimaxcmgui.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "threadcontrol.h"

ThreadControl::ThreadControl(QObject *parent): QThread(parent) {}

ThreadControl::~ThreadControl() {}

void ThreadControl::run()
{
    int rssi, cinr, dl, ul;
    bool waiting, connected, nodevice;
    waiting = connected = nodevice = false; // Thanks to Ujjal Vai for this great support!
    forever
    {
        char * info = (char*)malloc(50);
        FILE* wimaxcstatus = popen("wimaxc status | egrep \"Link|NOT\"", "r");
        {
            fread(info, 1, 50, wimaxcstatus);
            pclose(wimaxcstatus);

            if(strstr(info, "WAIT")) // If Device Is Plugged In But Not Connected
                {
                    if(!waiting)
                        {
                            emit SendDeviceStatus(1);
                            waiting = true;
                            connected = false;
                            nodevice = false;
                        }
                } // End

            else if(strstr(info, "LINKUP")) // If Device Is Plugged And Connected
                {
                    if(!connected)
                        {
                            emit SendDeviceStatus(2);
                            waiting = false;
                            connected = true;
                            nodevice = false;
                        }

                    FILE* linkstats = popen("wimaxc linkstats | egrep \"RSSI|CINR|kbps\"", "r");
                    fscanf(linkstats, "CINR (dB) %d\nRSSI (dBm) %d\nDL rate (kbps) %d\nUL rate (kbps) %d", &cinr, &rssi, &dl, &ul);
                    emit SendLinkStatus(rssi, cinr, dl, ul);
                    pclose(linkstats);
                }

            else if(strstr(info,"NOT")) // If Device Is Unplugged
                {
                    if(!nodevice)
                        {
                            emit SendDeviceStatus(0);
                            waiting = false;
                            connected = false;
                            nodevice = true;
                        }
                }
        } // End of reading file is exists
        free(info);
        msleep(2000);
    }  // End of forever
}
