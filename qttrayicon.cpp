/*
 * Copyright 2013 Marco Gulino <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <Wt/WServer>

#include "qttrayicon.h"
#include <QtGui/qicon.h>
#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/qmenu.h>
#include <QtCore/qtimer.h>
#include <QtCore/QProcess>

QtTrayIcon::~QtTrayIcon()
{
  delete systemTray->contextMenu();
  delete systemTray;
}



QtTrayIcon::QtTrayIcon(Wt::WServer& wserver, QObject* parent): QObject(parent), wserver(wserver), systemTray(new QSystemTrayIcon{QIcon::fromTheme("pandorica")})
{
  systemTray->setToolTip("Pandorica");
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(appClosed()));
  QTimer::singleShot(1000, this, SLOT(appStarted()));
  QMenu *menu = new QMenu{"Pandorica"};
  menu->addAction("Open in a browser window", this, SLOT(openInBrowser()));
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

void QtTrayIcon::openInBrowser()
{
  QString url = QString("http://localhost:%1/").arg(wserver.httpPort());
  QProcess::execute("xdg-open", {url});
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

void QtTrayIcon::appStarted()
{
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(checkServerStatus()));
  timer->start(500);
  try {
    wserver.start();
  } catch(std::exception &e) {
    closing = true;
    std::cerr << e.what();
    connect(systemTray, SIGNAL(messageClicked()), qApp, SLOT(quit()));
    systemTray->showMessage("Error", "There was an error starting the webserver.\nSee console output for details", QSystemTrayIcon::Critical, 10000);
    QTimer::singleShot(10000, qApp, SLOT(quit()));
  }
}

