/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pandoricawizard.h"
#include "pandoricawizard_p.h"
#include "utils/d_ptr_implementation.h"
#include "Models/setting.h"
#include "selectdirectories.h"
#include "settings.h"
#include "session.h"
#include <Wt/WGroupBox>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WPushButton>
#include <Wt/WLabel>
#include <Wt/WToolBar>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>
#include "Wt-Commons/wt_helpers.h"
#include "Models/models.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;
PandoricaWizard::Private::Private(PandoricaWizard* q) : q(q), stack(new WStackedWidget)
{
}

PandoricaWizard::Private::~Private()
{
}


void PandoricaWizard::Private::addPandoricaModePage()
{
  auto currentDatabaseType = Settings::databaseType();
  Settings::PandoricaMode currentMode = Settings::pandoricaMode();
  auto pandoricaMode = new WGroupBox("Pandorica Mode");
  
  WButtonGroup *buttonGroup = new WButtonGroup(pandoricaMode);
  WRadioButton *simpleMode, *advancedMode;
  buttonGroup->addButton(simpleMode = WW<WRadioButton>("simple mode").setInline(false));
  buttonGroup->addButton(advancedMode = WW<WRadioButton>("advanced mode").setInline(false));
  simpleMode->setChecked(currentMode == Settings::Simple);
  advancedMode->setChecked(currentMode == Settings::Advanced);
  pandoricaMode->addWidget(simpleMode);
  WLabel *simpleModeLabel = WW<WLabel>("Sigle user mode, automatically share every content to anyone on the same network without permissions or logging in. This is suggested for regular home usage.").setInline(false);
  simpleModeLabel->setBuddy(simpleMode);
  pandoricaMode->addWidget(simpleModeLabel);
  WLabel *advancedModeLabel = WW<WLabel>("Multi user mode, requires login and permissions for viewing content. Highly reccomended for public network servers.").setInline(false);
  pandoricaMode->addWidget(advancedMode);
  advancedModeLabel->setBuddy(advancedMode);
  pandoricaMode->addWidget(advancedModeLabel);
  
  buttonGroup->checkedChanged().connect([=](WRadioButton* b,_n5){
    Settings::PandoricaMode newMode = b==simpleMode ? Settings::Simple : Settings::Advanced;
    Settings::databaseType(newMode == Settings::Simple ? Settings::Sqlite3 : currentDatabaseType);
    next->setEnabled(true);
    Settings::pandoricaMode(newMode);
  });
  stack->addWidget(pandoricaMode);
  showPage[PandoricaModePage] = [=] {
    stack->setCurrentWidget(pandoricaMode);
    next->setEnabled(Settings::pandoricaMode() != Settings::Unset);
    previous->setDisabled(true);
    finish->setDisabled(true);
    nextPage = FileSystemChooserPage;
  };
}


void PandoricaWizard::Private::addFileSystemChooser()
{
  auto fileSystemChooser = new WGroupBox("Multimedia Library");
  auto selectDirectories = new SelectDirectories({"/"},
						 Settings::mediasDirectories(), [=](const string &p){Settings::addMediaDirectory(p);},
						 [=](const string &p){Settings::removeMediaDirectory(p);},
						 SelectDirectories::Multiple );
  selectDirectories->setHeight(500);
  selectDirectories->addTo(fileSystemChooser);
  stack->addWidget(fileSystemChooser);
  showPage[FileSystemChooserPage] = [=] {
    stack->setCurrentWidget(fileSystemChooser);
    next->setEnabled(Settings::pandoricaMode() != Settings::Simple);
    previous->setEnabled(true);
    finish->setEnabled(Settings::pandoricaMode() == Settings::Simple);
    nextPage = DatabaseSetup;
    previousPage = PandoricaModePage;
  };
}

void PandoricaWizard::Private::addDatabaseSetup()
{
  auto dbType = Settings::databaseType();
  auto groupBox = new WGroupBox("Setup Database Connection");
  WButtonGroup *buttonGroup = new WButtonGroup(groupBox);
  WRadioButton *sqlite3, *postgresql;
  buttonGroup->addButton(sqlite3 = WW<WRadioButton>("Sqlite3").setInline(false));
  buttonGroup->addButton(postgresql = WW<WRadioButton>("PostgreSQL").setInline(false));
  sqlite3->setChecked(dbType == Settings::Sqlite3);
  postgresql->setChecked(dbType == Settings::PostgreSQL);
  groupBox->addWidget(sqlite3);
  WLabel *sqlite3Label = WW<WLabel>("A simple, single file database, requiring no setup. Reccomended for few users usage.").setInline(false);
  sqlite3Label->setBuddy(sqlite3);
  groupBox->addWidget(sqlite3Label);
  WLabel *postgresqlLabel = WW<WLabel>("A more advanced database engine. Requires an external PostgreSQL installation.").setInline(false);
#ifndef HAVE_POSTGRES
  postgresql->setEnabled(false);
#endif
  groupBox->addWidget(postgresql);
  postgresqlLabel->setBuddy(postgresql);
  stack->addWidget(groupBox);
  groupBox->addWidget(postgresqlLabel);
  showPage[DatabaseSetup] = [=] {
    previous->setEnabled(true);
    next->setEnabled(true);
    finish->setEnabled(false);
    previousPage = FileSystemChooserPage;
    stack->setCurrentWidget(groupBox);
  };
}


GroupPtr PandoricaWizard::Private::adminGroup()
{
  Session session;
  Dbo::Transaction t(session);
  GroupPtr adminGroup = session.find<Group>().where("group_name = ?").bind("Admin");
  wApp->log("notice") << "Admin group found: " << adminGroup << (adminGroup ? string{": "} + adminGroup->groupName() : string{});
  if(! adminGroup) {
    wApp->log("notice") << "Creating main admin group";
    adminGroup = session.add(new Group("Admin", true));
  }
}


PandoricaWizard::~PandoricaWizard()
{
  wApp->log("notice") << "Finalizing settings...";
  Session session(true);
  Dbo::Transaction t(session);
  auto adminGroup = d->adminGroup();
  
  Auth::User adminAuthUser = session.users().findWithIdentity("pandorica", "Admin");
  if(!adminAuthUser.isValid()) {
    adminAuthUser = session.users().registerNew();
    session.users().setIdentity(adminAuthUser, "pandorica", "Admin");
    AuthInfoPtr authInfo = session.users().find(adminAuthUser);
    UserPtr adminUser = session.add(new User());
    adminAuthUser.setStatus(Auth::User::Normal);
    authInfo.modify()->setUser(adminUser);
    adminGroup.modify()->users.insert(adminUser);
    t.commit();
  }
  Setting::write(Setting::PandoricaSetup, true);
  d->finished.emit();
}

Signal<>& PandoricaWizard::finished() const
{
  return d->finished;
}


PandoricaWizard::PandoricaWizard(Wt::WContainerWidget* parent)
    : d(this)
{
  setImplementation(
    WW<WContainerWidget>().add(d->stack)
    .add(
      WW<WToolBar>().css("pull-right")
      .addButton(d->previous = WW<WPushButton>(WString::tr("button.previous")).css("btn-warning").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->previousPage](); }))
      .addButton(d->next = WW<WPushButton>(WString::tr("button.next")).css("btn-primary").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->nextPage](); }))
      .addButton(d->finish = WW<WPushButton>(WString::tr("button.finish")).css("btn-success").setEnabled(false).onClick([=](WMouseEvent){ delete this; }))
    )
  );
  d->addPandoricaModePage();
  d->addFileSystemChooser();
  d->addDatabaseSetup();
  d->showPage[Private::PandoricaModePage]();
}
