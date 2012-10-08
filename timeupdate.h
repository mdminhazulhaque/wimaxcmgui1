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

#ifndef TIMEUPDATE_H
#define TIMEUPDATE_H

#include <QThread>
#include <QTime>
#include <cstdio>

class TimeUpdate : public QThread
{
    Q_OBJECT
public:
    explicit TimeUpdate(QObject *parent = 0);
    ~TimeUpdate();
    void run();
    void  stop();

private:
    bool isActive;

signals:
    void SendTime(QTime);
};

#endif // TIMEUPDATE_H
