/***********************************************************************
Copyright (c) 2013 "Marco Gulino <marco.gulino@gmail.com>"

This file is part of Pandorica: https://github.com/GuLinux/Pandorica

Pandorica is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (included the COPYING file).

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/



#include <Wt/WServer>

#include "qttrayicon.h"
#include "utils.h"
#include <QtGui/qicon.h>
#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMenu>
#include <QtGui/QClipboard>
#include <QtCore/qtimer.h>
#include <QtNetwork/QNetworkInterface>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

QtTrayIcon::~QtTrayIcon()
{
  delete systemTray->contextMenu();
  delete systemTray;
}



QtTrayIcon::QtTrayIcon(Wt::WServer& wserver, QObject* parent): QObject(parent), wserver(wserver), 
  systemTray(new QSystemTrayIcon{
#ifdef WIN32
    QIcon(":/tray-icon.png")
#else
    QIcon::fromTheme("pandorica")
#endif
  })
{
  systemTray->setToolTip("Pandorica");
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(appClosed()));
  QTimer::singleShot(1000, this, SLOT(appStarted()));
  QMenu *menu = new QMenu{"Pandorica"};
  menu->addAction("Open in a browser window", this, SLOT(openInBrowser()));
  menu->addAction("Copy link address", this, SLOT(copyLinkAddress()));
  menu->addAction("Quit", qApp, SLOT(quit()));
  systemTray->setContextMenu(menu);
  systemTray->show();
  connect(systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(activated(QSystemTrayIcon::ActivationReason)));
}

void QtTrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
  if( (reason == QSystemTrayIcon::DoubleClick) && wserver.isRunning())
    openInBrowser();
}

QString QtTrayIcon::httpAddress() const
{
  return QString("http://%1:%2/").arg(networkAddress).arg(wserver.httpPort());
}


void QtTrayIcon::openInBrowser()
{
  QDesktopServices::openUrl({httpAddress()});
}


void QtTrayIcon::appClosed()
{
  if(wserver.isRunning())
    wserver.stop();
}

void QtTrayIcon::checkServerStatus()
{
  if(!started && wserver.isRunning()) {
    systemTray->showMessage("Pandorica", QString("Pandorica web server running at port %1").arg(wserver.httpPort()));
    started = true;
    return;
  }
  if(!wserver.isRunning() && ! closing) {
    closing = true;
    connect(systemTray, SIGNAL(messageClicked()), qApp, SLOT(quit()));
    systemTray->showMessage("Error", "Pandorica web stopped, closing application.", QSystemTrayIcon::Critical, 10000);
    QTimer::singleShot(10000, qApp, SLOT(quit()));
  }
}

void QtTrayIcon::copyLinkAddress()
{
  QApplication::clipboard()->setText(httpAddress());
}


void QtTrayIcon::appStarted()
{
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(checkServerStatus()));
  timer->start(500);
  try {
    networkAddress = "localhost";
    for(QString addr: Utils::transform(QNetworkInterface::allAddresses(), std::vector<QString>{}, [](QHostAddress a){ return a.toString(); }) ) {
      if(addr.startsWith("192.168.") || addr.startsWith("10.0.")) {
        networkAddress = addr;
        break;
      }
    }
    wserver.start();
  } catch(std::exception &e) {
    closing = true;
    std::cerr << e.what();
    connect(systemTray, SIGNAL(messageClicked()), qApp, SLOT(quit()));
    systemTray->showMessage("Error", "There was an error starting the webserver.\nSee console output for details", QSystemTrayIcon::Critical, 10000);
    QTimer::singleShot(10000, qApp, SLOT(quit()));
  }
}

