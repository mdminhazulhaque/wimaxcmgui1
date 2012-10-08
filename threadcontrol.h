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

#ifndef THREADCONTROL_H
#define THREADCONTROL_H

#include <QThread>
#include <cstring>
#include <cstdio>
#include <cstdlib>

class ThreadControl : public QThread
{
    Q_OBJECT
public:
    ThreadControl(QObject *parent = 0);
    ~ThreadControl();
    void run();

signals:
    void SendDeviceStatus(int);
    void SendLinkStatus(int, int, int, int); // rssi, cinr, dl, ul
};

#endif // THREADCONTROL_H
