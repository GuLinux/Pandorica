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




#include "serversettingspage.h"
#include "private/serversettingspage_p.h"
#include "settings.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include "media/mediacollection.h"
#include "selectdirectories.h"
#include "Models/setting.h"
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WInPlaceEdit>
#include <Wt/WGroupBox>
#include <Wt/WCheckBox>
#include <Wt/WTemplate>
#include <Wt/WImage>
#include <Wt/WIOService>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "utils/utils.h"
#include "utils/d_ptr_implementation.h"



using namespace std;
using namespace Wt;
using namespace WtCommons;

namespace fs = boost::filesystem;

ServerSettingsPage::Private::Private(Settings* settings, Session* session, MediaCollection *mediaCollection, ServerSettingsPage* q)
  : settings(settings), session(session), mediaCollection(mediaCollection), q(q)
{
}

ServerSettingsPage::~ServerSettingsPage()
{
}

ServerSettingsPage::ServerSettingsPage(Settings* settings, Session* session, MediaCollection *mediaCollection, WObject* parent)
    : WDialog(parent), d(settings, session, mediaCollection, this)
{
  setWindowTitle(wtr("menu.configure.app"));
  contents()->addStyleClass("server-settings-page-body");
  contents()->addWidget(d->selectMediaRootPage());
  setClosable(false);
//   setHeight(650);
//   setWidth(600);

  footer()->addWidget(d->buttonOk = WW<WPushButton>(wtr("button.ok")).css("btn btn-primary").onClick([=](WMouseEvent) { accept(); }));
  finished().connect([=](WDialog::DialogCode, _n5) {
    mediaCollection->rescan([]{} );
  });
}

WContainerWidget* ServerSettingsPage::Private::selectMediaRootPage()
{
  WGroupBox *groupBox = WW<WGroupBox>(wtr("configure.app.select_media_directories")).css("fieldset-small");
  vector<string> rootPaths;
#ifdef WIN32
  for(char driveLetter = 0x41; driveLetter <= 0x5A; driveLetter++) {
    string drive = (boost::format("%s:/") % driveLetter).str();
    if(boost::filesystem::exists(boost::filesystem::path{drive}))
      rootPaths.push_back(drive);
  }
#else
  rootPaths.push_back("/");
#endif
  SelectDirectories *selectDirectories = new SelectDirectories(rootPaths, settings->mediasDirectories(), [=](string p){
    settings->addMediaDirectory(p);
  }, [=](string p){
    settings->removeMediaDirectory(p);
  }, SelectDirectories::Multiple, groupBox );
  selectDirectories->setHeight(400);
  return groupBox;
}

void ServerSettingsPage::dialog(Settings* settings, Session* session, MediaCollection* mediaCollection, WObject* parent)
{
  (new ServerSettingsPage(settings, session, mediaCollection, parent))->show();
}

