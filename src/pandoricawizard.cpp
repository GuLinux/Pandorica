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
  auto pandoricaMode = new WGroupBox(WString::tr("wizard.pandoricamode"));
  
  WButtonGroup *buttonGroup = new WButtonGroup(pandoricaMode);
  WRadioButton *simpleMode = addRadio("wizard.pandoricamode.simple", pandoricaMode, buttonGroup);
  WRadioButton *advancedMode = addRadio("wizard.pandoricamode.advanced", pandoricaMode, buttonGroup);
  
  buttonGroup->checkedChanged().connect([=](WRadioButton* b,_n5){
    Settings::PandoricaMode newMode = b==simpleMode ? Settings::Simple : Settings::Advanced;
    Settings::databaseType(newMode == Settings::Simple ? Settings::Sqlite3 : currentDatabaseType);
    Settings::pandoricaMode(newMode);
  });
  stack->addWidget(pandoricaMode);
  showPage[PandoricaModePage] = [=] {
    displayPage(pandoricaMode, None, FileSystemChooserPage);
    buttonGroup->setCheckedButton(Settings::pandoricaMode() == Settings::Simple ? simpleMode : advancedMode);
  };
}


void PandoricaWizard::Private::addFileSystemChooser()
{
  auto fileSystemChooser = new WGroupBox(WString::tr("wizard.medialibrary"));
  auto selectDirectories = new SelectDirectories({"/"},
						 Settings::mediasDirectories(), [=](const string &p){Settings::addMediaDirectory(p);},
						 [=](const string &p){Settings::removeMediaDirectory(p);},
						 SelectDirectories::Multiple );
  selectDirectories->setHeight(500);
  selectDirectories->addTo(fileSystemChooser);
  stack->addWidget(fileSystemChooser);
  showPage[FileSystemChooserPage] = [=] {
    displayPage(fileSystemChooser, PandoricaModePage, Settings::pandoricaMode() == Settings::Simple ? None : DatabaseSetup);
  };
}

void PandoricaWizard::Private::addDatabaseSetup()
{
  auto dbType = Settings::databaseType();
  auto groupBox = new WGroupBox(WString::tr("wizard.dbsettings"));
  WButtonGroup *buttonGroup = new WButtonGroup(groupBox);
  WRadioButton *sqlite3 = addRadio("wizard.sqlite3", groupBox, buttonGroup);
  WRadioButton *postgresql = addRadio("wizard.postgresql", groupBox, buttonGroup);
  
#ifndef HAVE_POSTGRES
  postgresql->setEnabled(false);
#endif
  stack->addWidget(groupBox);
  showPage[DatabaseSetup] = [=] {
    displayPage(groupBox, FileSystemChooserPage, None);
  };
}


void PandoricaWizard::Private::displayPage(WWidget* widget, Page previousPage, Page nextPage)
{
  stack->setCurrentWidget(widget);
  this->previousPage = previousPage;
  this->nextPage = nextPage;
  previous->setEnabled(previousPage != None);
  next->setEnabled(nextPage != None);
  finish->setEnabled(nextPage == None);
}


WRadioButton* PandoricaWizard::Private::addRadio(const string& textKey, WContainerWidget* container, WButtonGroup* buttonGroup)
{
  WRadioButton *button = WW<WRadioButton>(WString::tr(textKey)).setInline(false);
  WLabel *label = WW<WLabel>(WString::tr(textKey + ".label")).setInline(false);
  WW<WContainerWidget>(container).add(button).add(label);
  label->setBuddy(button);
  buttonGroup->addButton(button);
  return button;
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
