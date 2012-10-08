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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QDir>
#include <cstdlib>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QMainWindow>
#include <QDesktopServices>

#include "timeupdate.h"
#include "threadcontrol.h"
#include "ui_mainwindow.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void CheckWiMAXService();
    void SetStatusConnected();
    void SetStatusDisconnected();
    void SetStatusNoDevice();
    void ResetAllValues();
    void SetBSID();
    void SetIP();
    void StatusTabJob();
    void AccountTabJob();
    void CheckUIDs();
    void CheckEthAlias();
    void SetEnvironmentVariables();

private slots:

    //////////////////////////////////// Custom Slots ////////////////////////////////////

    void ReceiveLinkStatus(int, int, int, int); // RSSI, CINR, DL, UL
    void ReceiveDeviceStatus(int);
    void ReceiveTime(QTime);

    //////////////////////////////////// UI Slots ////////////////////////////////////

    void on_buttonClose_clicked();
    void on_buttonCD_clicked();
    void on_buttonLoadMAC_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_buttonActivateProfile_clicked();
    void on_buttonSaveProfile_clicked();
    void on_buttonLoadProfile_clicked();
    void on_buttonRemoveProfile_clicked();
    void on_buttonEthUp_clicked();
    void on_buttonManualConnect_clicked();
    void on_buttonOpenHelp_clicked();
    void on_buttonRefreshIP_clicked();
    void on_buttonConsole_clicked();
    void on_buttonConfigure_clicked();
    void on_buttonSetBSID_clicked();
    void on_comboProfile_activated(const QString &currenttext);

private:

    Ui::MainWindow *ui;
    ThreadControl *wControl;
    TimeUpdate *wTime;

    QStringList UserIDList;
    QString GUIRoot;
    QString Ethernet;
    QString HWAddress;
    QString Terminal;
    bool isConnected;
};

#endif // MAINWINDOW_H
