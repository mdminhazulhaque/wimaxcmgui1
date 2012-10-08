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

#include "timeupdate.h"


TimeUpdate::TimeUpdate(QObject *parent) : QThread(parent) {}

TimeUpdate::~TimeUpdate() {}

void TimeUpdate::run()
{
    isActive = true;
    int h, m, s;

    // Read hh:mm:ss from pipe

    FILE *connectiontime = popen("wimaxc status | grep Connection | cut -d' ' -f3","r");
    fscanf (connectiontime,"%d:%d:%d", &h, &m, &s);
    pclose(connectiontime);

    // End reading

    QTime time(h, m, s);
    while(isActive)
    {
        emit SendTime(time);
        time = time.addSecs(1);
        msleep(1000);
    }
}

void TimeUpdate::stop()
{
    isActive = false;
}

