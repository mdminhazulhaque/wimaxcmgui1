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

#include "mainwindow.h"

/**************************
******* Constructor *******
**************************/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // UI Setup
    ui->setupUi(this);
    this->setFixedSize(430, 340);
    ui->links->setOpenExternalLinks(1);

    // Initial Check
    CheckWiMAXService();
    SetEnvironmentVariables();
    CheckUIDs();

    wControl = new ThreadControl(this);
    wTime = new TimeUpdate(this);

    QObject::connect(wControl, SIGNAL(SendLinkStatus(int,int,int,int)), this, SLOT(ReceiveLinkStatus(int,int,int,int)));
    QObject::connect(wControl, SIGNAL(SendDeviceStatus(int)), this, SLOT(ReceiveDeviceStatus(int)));
    QObject::connect(wTime, SIGNAL(SendTime(QTime)), this, SLOT(ReceiveTime(QTime)));

    //Start the main thread
    wControl->start();
}

/**************************
******* Destructor ********
**************************/
MainWindow::~MainWindow()
{
    delete wControl;
    delete wTime;
    delete ui;
}

void MainWindow::CheckWiMAXService()
{
    char *buffer = (char*)malloc(50);
    FILE *wimaxdcheck = popen("ps -A | grep wimaxd", "r");
    fread(buffer, 1, 50, wimaxdcheck);
    if(strstr(buffer,"wimaxd"))
        return;
    else
        {
            pclose(wimaxdcheck);
            free(buffer);
            QMessageBox::warning(this, "No Service", "WiMAX service is not running.\nCheck the manual for details");
            exit(0);
        }
    pclose(wimaxdcheck);
    free(buffer);
}

/*************************************
***** Set Environment Variables ******
*************************************/
void MainWindow::SetEnvironmentVariables()
{
    if(QFile::exists("/usr/bin/kdesu"))
        GUIRoot = "kdesu";
    else
        GUIRoot = "gksu";


    if(QFile::exists("/usr/bin/xterm"))
        Terminal = "xterm";
    else if(QFile::exists("/usr/bin/konsole"))
        Terminal = "konsole";
    else if(QFile::exists("/usr/bin/lxterminal"))
        Terminal = "lxterminal";
    else if(QFile::exists("/usr/bin/gnome-terminal"))
        Terminal = "gnome-terminal";
    else
        Terminal = "uxterm";
}

/**************************
***** Get Saved UIDs ******
**************************/
void MainWindow::CheckUIDs()
{
    FILE *userdatafile = fopen(QString(QDir::homePath() + "/.wimaxsavedidentities").toAscii(),"r");
    if(userdatafile != NULL)
        {
            char *uid, *passwd, *hwaddr;
            uid = (char*)malloc(50);
            passwd = (char*)malloc(50);
            hwaddr = (char*)malloc(20);

            // Read from file
            while (fscanf (userdatafile, "%s %s %s", uid, passwd, hwaddr) != EOF)
                {
                    UserIDList.append(QString(uid));
                    UserIDList.append(QString(passwd));
                    UserIDList.append(QString(hwaddr));
                }
            fclose(userdatafile);
            free(uid);
            free(passwd);
            free(hwaddr);

            if(UserIDList.isEmpty())
                qDebug("No saved userdata found!");
            else
                //  And add to combo box
                for (int i=0 ; i<UserIDList.size(); i+=3)
                    ui->comboProfile->addItem(UserIDList.at(i));
        }
}

/**************************
******* Check Alias *******
**************************/
void MainWindow::CheckEthAlias()
{
    char *MACAddress = (char*)malloc(20);
    char *wimaxcmacaddr = (char*)malloc(20);
    char *ethernet = (char*)malloc(5);
    FILE *ifconfig;

    // Read MAC From wimaxc
    ifconfig = popen("wimaxc versions | grep MAC | cut -d' ' -f3","r");
    fscanf (ifconfig,"%s",wimaxcmacaddr);
    pclose(ifconfig);

    // Replace - with :
    HWAddress = QString::fromAscii(wimaxcmacaddr);
    HWAddress.replace("-",":");

    // Find ethX from $ ifconfig
    ifconfig = popen(QString("/sbin/ifconfig -a | grep -i %1 | cut -d' ' -f1").arg(HWAddress).toAscii(), "r");
    fscanf (ifconfig,"%s",ethernet);
    pclose(ifconfig);

    Ethernet = QString::fromAscii(ethernet);

    free(MACAddress);
    free(wimaxcmacaddr);
    free(ethernet);
}

/****************************************
****** Set All Values From Signals ******
****************************************/
void MainWindow::ReceiveLinkStatus(int rssi, int cinr, int dl, int ul)
{
    ui->statusRSSI->setText(QString::number(rssi)+" dBm");
    ui->statusCINR->setText(QString::number(cinr)+" dB");
    ui->statusDL->setText(QString::number(dl)+" kbps");
    ui->statusUL->setText(QString::number(ul)+" kbps");

    // Check by CINR
    if(cinr > 32)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/5"));
    else if(cinr > 24)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/4"));
    else if(cinr > 16)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/3"));
    else if(cinr > 8)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/2"));
    else
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/1"));

    /*
    Check by RSSI
    Will be implimented as customizable in next version

    if(rssi > -20)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/5"));
    else if(rssi > -40)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/4"));
    else if(rssi > -60)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/3"));
    else if(rssi > -80)
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/2"));
    else
        ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/1"));
    */

}

/*******************************
******* Reset All Values *******
*******************************/
void MainWindow::ResetAllValues()
{
    isConnected = false;
    wTime->stop();
    ui->statusTime->setText("--:--:--");
    ui->statusIP->setText("---.---.---.---");
    ui->statusBSID->setText("--:--:--:--:--:--");
    ui->statusRSSI->setText("--- dBm");
    ui->statusCINR->setText("-- dB");
    ui->statusDL->setText("--- kbps");
    ui->statusUL->setText("--- kbps");
}

/***********************************
******* Set Time From Signal *******
***********************************/
void MainWindow::ReceiveTime(QTime time)
{
    ui->statusTime->setText(time.toString("hh:mm:ss"));
}

/*******************************
******* Connected Status *******
*******************************/
void MainWindow::SetStatusConnected()
{
    // Changing Title
    this->setWindowTitle("WiMAX CM GUI: Connected");
    this->setWindowIcon(QPixmap(":/images/connected"));

    CheckEthAlias();

    isConnected = true;

    // Init Connection Jobs
    wTime->start();
    SetBSID();
    SetIP();

    ui->statusConnectionState->setText("Connected");
    ui->StatusIcon->setPixmap(QPixmap(":/images/connected"));
    ui->buttonCD->setIcon(QPixmap(":/images/disconnect"));
    on_tabWidget_currentChanged(0);

    ui->buttonCD->setText("Disconnect!");
    ui->buttonCD->setEnabled(1);
    ui->buttonManualConnect->setEnabled(1);
    ui->buttonLoadMAC->setEnabled(1);
    ui->buttonEthUp->setEnabled(1);
    ui->buttonRefreshIP->setEnabled(1);
    ui->buttonSetBSID->setEnabled(1);
    ui->lineEditMAC->setEnabled(1);;
}

/*********************************
******* Disonnected Status *******
*********************************/
void MainWindow::SetStatusDisconnected()
{
    // Changing Title
    this->setWindowTitle("WiMAX CM GUI: Disconnected");
    this->setWindowIcon(QPixmap(":/images/disconnected"));

    CheckEthAlias();

    isConnected = false;

    // Init Disconnection Jobs
    ResetAllValues();

    ui->statusConnectionState->setText("Disconnected");
    ui->StatusIcon->setPixmap(QPixmap(":/images/disconnected"));
    ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/0"));
    ui->statusBar->showMessage("Disconnected. Waiting for connection...");



    // Resetting The Network Indicator
    ui->buttonCD->setIcon(QPixmap(":/images/connect"));

    ui->buttonCD->setText("Connect!");
    ui->buttonCD->setEnabled(1);
    ui->buttonManualConnect->setEnabled(1);
    ui->buttonEthUp->setEnabled(1);
    ui->buttonLoadMAC->setEnabled(1);
    ui->buttonRefreshIP->setEnabled(0);
    ui->buttonSetBSID->setEnabled(0);
    ui->lineEditMAC->setEnabled(1);
}

/*****************************
****** No Device Status ******
*****************************/
void MainWindow::SetStatusNoDevice()
{
    this->setWindowIcon(QPixmap(":/images/nodevice"));
    this->setWindowTitle("WiMAX CM GUI: No Device");

    ResetAllValues();

    isConnected = false;

    ui->statusConnectionState->setText("No Device");
    ui->StatusIcon->setPixmap(QPixmap(":/images/nodevice"));
    ui->NetworkQualityMeter->setPixmap(QPixmap(":/images/0"));
    ui->buttonCD->setIcon(QPixmap(":/images/connect"));
    ui->statusBar->showMessage("No device. Plug it in...");

    ui->buttonCD->setText("Connect!");
    ui->buttonCD->setEnabled(0);
    ui->buttonManualConnect->setEnabled(0);
    ui->buttonLoadMAC->setEnabled(0);
    ui->buttonEthUp->setEnabled(0);
    ui->buttonRefreshIP->setEnabled(0);
    ui->lineEditMAC->setEnabled(0);
    ui->buttonSetBSID->setEnabled(0);
}

/**************************
* Signal Of Device Status *
**************************/
void MainWindow::ReceiveDeviceStatus(int status)
{
    switch(status)
        {
        case 1:
            SetStatusDisconnected();
        break;
        case 2:
            SetStatusConnected();
        break;
        case 0:
            SetStatusNoDevice();
        break;
        }
}

/*********************
****** Set BSID ******
*********************/
void MainWindow::SetBSID()
{
    char *BaseStationID = (char*)malloc(50);
    FILE *wimaxcbsid = popen("wimaxc status | grep Base | cut -d':' -f2-6","r");
    fscanf (wimaxcbsid,"%s",BaseStationID);
    pclose(wimaxcbsid);
    ui->statusBSID->setText(BaseStationID);
    free(BaseStationID);
}

/*********************
******* Set IP *******
*********************/
void MainWindow::SetIP()
{
    int *iparray = (int*)malloc(16);
    FILE *inetaddr;
    inetaddr = popen(QString("/sbin/ifconfig %1 | grep 'inet addr:' | cut -d: -f2 | awk '{print $1}'").arg(Ethernet).toAscii(), "r");
    fscanf(inetaddr, "%d.%d.%d.%d", &iparray[0], &iparray[1], &iparray[2], &iparray[3]);
    pclose(inetaddr);

    if(iparray[0] >= 0 && iparray[1] >= 0  && iparray[2] >= 0 && iparray[3] >= 0 && iparray[0] <= 255 && iparray[1] <= 255 && iparray[2] <= 255 && iparray[3] <= 255)
        ui->statusIP->setText(QString("%1.%2.%3.%4").arg(iparray[0]).arg(iparray[1]).arg(iparray[2]).arg(iparray[3]));
    else
        ui->statusIP->setText("---.---.---.---");
    free(iparray);
}

/*******************************
***** Status Bar: Main Tab *****
*******************************/
void MainWindow::StatusTabJob()
{
    if(isConnected)
        {
            unsigned int freq;
            FILE *wimaxcfreq = popen("wimaxc status | grep Freq | cut -d' ' -f4", "r");
            fscanf(wimaxcfreq, "%d", &freq);
            pclose(wimaxcfreq);
            ui->statusBar->showMessage(QString("Connected to %1MHz").arg(freq/1000));
        }
    else
        ui->statusBar->showMessage("There is no active connection");
}

/************************************
****** Status Bar: Account Tab ******
************************************/
void MainWindow::AccountTabJob()
{
    char *ActiveUserID = (char*)malloc(50);
    FILE *wimaxcuid = popen("wimaxc get UserIdentity | grep UserIdentity","r");
    fscanf (wimaxcuid,"UserIdentity %s",ActiveUserID);
    pclose(wimaxcuid);
    if(strcmp(ActiveUserID,"''") == 0)
        ui->statusBar->showMessage("No User ID is activated");
    else
        ui->statusBar->showMessage("Current User ID: "+QString(ActiveUserID));
    free(ActiveUserID);
}

/**************************************
****** Connect/Disconnect Button ******
**************************************/
void MainWindow::on_buttonCD_clicked()
{
    if(isConnected)
        system("wimaxc disconnect");
    else
        {
            QProcess::startDetached("wimaxautoconnect");
            ui->statusBar->showMessage("Searching ...", 5000);
        }
}

/********************************
****** Activate Button Job ******
********************************/
void MainWindow::on_buttonActivateProfile_clicked()
{
    if ( ui->lineEditUID->text() != QString("") && ui->lineEditPasswd->text() != QString("") && ui->lineEditMAC->text() != QString(""))
        {
            if(ui->lineEditMAC->text().contains(" ") || ui->lineEditPasswd->text().contains(" ") || ui->lineEditUID->text().contains(" "))
                {
                    QMessageBox::warning(this, "Error!", "Do not put white spaces in text");
                    return;
                }
            else
                {
                    system(QString("wimaxuserconfig %1 %2 %3").arg(ui->lineEditUID->text()).arg(ui->lineEditPasswd->text()).arg(ui->lineEditMAC->text()).toAscii());
                    on_tabWidget_currentChanged(1);
                    QMessageBox::information(this, "Success!", "Configuration saved!");
                }
        }
    else
        QMessageBox::warning(this, "Error!", "Please fill all entries");
}

/************************************
****** Save Profile Button Job ******
************************************/
void MainWindow::on_buttonSaveProfile_clicked()
{
    if ( ui->lineEditUID->text() != QString("") && ui->lineEditPasswd->text() != QString("") && ui->lineEditMAC->text() != QString(""))
        {
            if(ui->lineEditMAC->text().contains(" ") || ui->lineEditPasswd->text().contains(" ") || ui->lineEditUID->text().contains(" "))
                {
                    QMessageBox::warning(this, "Error!", "Do not put white spaces in text");
                    return;
                }
            else
                {
                    UserIDList.append( ui->lineEditUID->text() );
                    UserIDList.append( ui->lineEditPasswd->text() );
                    UserIDList.append( ui->lineEditMAC->text() );
                    ui->comboProfile->addItem(ui->lineEditUID->text());
                    ui->comboProfile->clear();
                    // Reload combo box

                    for (int i=0 ; i<UserIDList.size(); i+=3)
                        ui->comboProfile->addItem(UserIDList.at(i));

                    // Save to file

                    FILE *saveuserdata = fopen(QString(QDir::homePath() + "/.wimaxsavedidentities").toAscii(),"w");
                    for(int i = 0; i < UserIDList.size(); i+=3)
                        fputs(UserIDList.at(i).toAscii()+ " " + UserIDList.at(i+1).toAscii() + " " + UserIDList.at(i+2).toAscii() + "\n", saveuserdata);
                    fclose(saveuserdata);

                    // Save complete
                }
        }
    else
        QMessageBox::warning(this, "Error!", "Please fill all entries");
}

/***************************
****** Load Saved UID ******
***************************/
void MainWindow::on_buttonLoadProfile_clicked()
{
    if(!UserIDList.empty())
        {
            QString currenttext = ui->comboProfile->currentText();
            for(int i=0;i<UserIDList.length();i+=3)
                if(UserIDList.at(i) == currenttext)
                    {
                        ui->lineEditUID->setText(UserIDList.at(i));
                        ui->lineEditPasswd->setText(UserIDList.at(i+1));
                        ui->lineEditMAC->setText(UserIDList.at(i+2));
                    }
        }
    else
        QMessageBox::warning(this, "Error!", "No profile in memory!");
}

/****************************
****** Load MAC Address *****
****************************/
void MainWindow::on_buttonLoadMAC_clicked()
{
    ui->lineEditMAC->setText(HWAddress.remove(":"));
}

/****************************
***** Delete Saved UIDs *****
****************************/
void MainWindow::on_buttonRemoveProfile_clicked()
{
    if(!UserIDList.empty())
        {
            // Brilliant Logic By Abu Asif Khan :D

            int index = ui->comboProfile->currentIndex();
            ui->comboProfile->removeItem(index);
            UserIDList.removeAt(index * 3);
            UserIDList.removeAt(index * 3);
            UserIDList.removeAt(index * 3);

            ui->comboProfile->clear();

            for (int i=0 ; i<UserIDList.size(); i+=3)
                ui->comboProfile->addItem(UserIDList.at(i));

            FILE *userdatafile = fopen(QString(QDir::homePath() + "/.wimaxsavedidentities").toAscii(),"w");
            for(int i = 0; i < UserIDList.size(); i+=3)
                fputs(UserIDList.at(i).toAscii()+ " " + UserIDList.at(i+1).toAscii() + " " + UserIDList.at(i+2).toAscii() + "\n", userdatafile);
            fclose(userdatafile);
        }
    else
        QMessageBox::warning(this, "Error!", "No profile in memory!");
}

/****************************
****** IFCONFIG ETH UP ******
****************************/
void MainWindow::on_buttonEthUp_clicked()
{
    switch(QMessageBox::question(this, "Confirm", QString("Device is found at %1\nDo you want to ifup it?").arg(Ethernet), QMessageBox::Yes, QMessageBox::No))
        {
        case QMessageBox::Yes:
            system(QString("%1 ifconfig %2 up").arg(GUIRoot).arg(Ethernet).toAscii());
        break;
        case QMessageBox::No:
        break;
        }
}

/****************************
****** Manual Connect *******
****************************/
void MainWindow::on_buttonManualConnect_clicked()
{
    switch(QMessageBox::question(this, "Confirm", QString("Opening manual connection console will\ndisconnect the active connection (if any).\nContinue?"), QMessageBox::Yes, QMessageBox::No))
        {
        case QMessageBox::Yes:
        break;
        case QMessageBox::No:
        return;
        }
    QProcess::startDetached(QString("%1 -e wimaxc -i search").arg(Terminal));
}

/*************************
********** Exit **********
*************************/
void MainWindow::on_buttonClose_clicked()
{
    if(isConnected)
        switch(QMessageBox::question(this, "Confirm", QString("Do you want to disconnect before closing?"), QMessageBox::Yes, QMessageBox::No))
            {
            case QMessageBox::Yes:
                system("wimaxc disconnect");
            break;
            case QMessageBox::No:
            break;
            }
    exit(0);
}

/*************************
******* Open Help ********
*************************/
void MainWindow::on_buttonOpenHelp_clicked()
{
    QMessageBox::warning(this, "Error!", "Help guide is not available in global release.");
    // QDesktopServices::openUrl(QUrl("/usr/share/wimaxcmgui/help.pdf"));
}

/*************************
******* IP Refresh *******
*************************/
void MainWindow::on_buttonRefreshIP_clicked()
{
    SetIP();
}

/**************************
******* Open Console ******
**************************/
void MainWindow::on_buttonConsole_clicked()
{
    //QProcess::startDetached("konsole -e wimaxc -i");
    QProcess::startDetached(QString("%1 -e wimaxc -i").arg(Terminal));
}

/**************************
******** Configure ********
**************************/
void MainWindow::on_buttonConfigure_clicked()
{
    switch(QMessageBox::question(this, "New Configuration", QString("Do you want to configure firmware and certificate?"), QMessageBox::Yes, QMessageBox::No))
        {
        case QMessageBox::Yes:
            // PEM BIN CFG
            system(QString("%1 wimaxfileconfig '%2' '%3' '%4'").arg(GUIRoot).arg(QFileDialog::getOpenFileName(this, "Select Certificate File", QDir::homePath(), tr("Certificate File (*.pem)"))).arg(QFileDialog::getOpenFileName(this, "Select Firmware", QDir::homePath(), tr("macxvi200.bin (macxvi200.bin)"))).arg(QFileDialog::getOpenFileName(this, "Select CFG", QDir::homePath(), tr("macxvi.cfg(macxvi.cfg)"))).toAscii());
            if(QFile::exists("/etc/wimaxcert.pem") && QFile::exists("/lib/firmware/macxvi200.bin") && QFile::exists("/lib/firmware/macxvi200.bin"))
                QMessageBox::information(this, "Successful!", "Files are updated successfully!");
            else
                QMessageBox::warning(this, "Error!", "Files update failed!");
        break;
        case QMessageBox::No:
        break;
        }
}

/*************************
****** BSID Refresh ******
*************************/
void MainWindow::on_buttonSetBSID_clicked()
{
    SetBSID();
}

/******************************
****** Tab Switch Signal ******
******************************/
void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(index)
        {
        case 0: StatusTabJob(); break;
        case 1: AccountTabJob(); break;
        case 2: statusBar()->showMessage("If you encounter any problem, open the help guide"); break;
        case 3: statusBar()->showMessage("A little piece of software written in Qt (C++)"); break;
        }
}

/**************************************************
****** Auto Load Info When Selecting Combo Box Items ******
**************************************************/
void MainWindow::on_comboProfile_activated(const QString &currenttext)
{
    for(int i=0;i<UserIDList.length();i+=3)
        if(UserIDList.at(i) == currenttext)
            {
                ui->lineEditUID->setText(UserIDList.at(i));
                ui->lineEditPasswd->setText(UserIDList.at(i+1));
                ui->lineEditMAC->setText(UserIDList.at(i+2));
            }
}
