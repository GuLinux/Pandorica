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



#ifndef SERVERSETTINGSDIALOGPRIVATE_H
#define SERVERSETTINGSDIALOGPRIVATE_H
#include <Wt/WLength>
#include <string>
#include "serversettingspage.h"

namespace Wt
{
  class WPushButton;
  class WContainerWidget;
  class WInteractWidget;
}

class MediaCollection;
class Session;
class Settings;
class ServerSettingsPage::Private
{
  public:
    Private( Settings *settings, Session *session, MediaCollection *mediaCollection, ServerSettingsPage *q );
    Settings *settings;
    Session *session;
    MediaCollection *mediaCollection;
    Wt::WPushButton *buttonOk;
    Wt::WPushButton *buttonNext;
    Wt::WContainerWidget *selectMediaRootPage();
    Wt::WContainerWidget *cachePage();
    void buildDeployTypePage();
    Wt::WContainerWidget *selectDeployTypeContainer;
  private:
    class ServerSettingsPage *const q;
};
#endif // SERVERSETTINGSDIALOGPRIVATE_H
