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
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/RegistrationModel>
#include <Wt/Auth/RegistrationWidget>
#include <Wt/WLineEdit>
#include <Wt/WSpinBox>
#include <Wt/WCheckBox>
#include "Wt-Commons/wt_helpers.h"
#include <Wt-Commons/wform.h>
#include <Wt-Commons/wobjectscope.h>
#include "Models/models.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;
PandoricaWizard::Private::Private(PandoricaWizard* q) : q(q), stack(WW<WStackedWidget>().addCss("wizard"))
{
}

PandoricaWizard::Private::~Private()
{
}


void PandoricaWizard::Private::addBeginPage()
{
  WGroupBox *installation = WW<WGroupBox>(WString::tr("wizard.begin")).add(WW<WText>(WString::tr("wizard.begin.message")));
  stack->addWidget(installation);
  showPage[Begin] = [=] {
    displayPage(installation, None, PandoricaModePage);
  };
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
    displayPage(pandoricaMode, Begin, FileSystemChooserPage);
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
  fileSystemChooser->addWidget(WW<WText>(WString::tr("wizard.medialibrary.message")));
  fileSystemChooser->addWidget(selectDirectories);
  stack->addWidget(fileSystemChooser);
  showPage[FileSystemChooserPage] = [=] {
    displayPage(fileSystemChooser, PandoricaModePage, Settings::pandoricaMode() == Settings::Simple ? None : DatabaseSetup);
  };
}

void PandoricaWizard::Private::addDatabaseSetup()
{
  auto groupBox = new WGroupBox(WString::tr("wizard.dbsettings"));
  WButtonGroup *buttonGroup = new WButtonGroup(groupBox);
  WRadioButton *sqlite3 = addRadio("wizard.sqlite3", groupBox, buttonGroup);
  WRadioButton *postgresql = addRadio("wizard.postgresql", groupBox, buttonGroup);
  WGroupBox *postgresqlSettings = WW<WGroupBox>(WString::tr("wizard.postgresql")).setHidden(true);
  
  WForm *postgresqlSettingsForm = WW<WForm>();
  WLineEdit *psqlUser, *psqlHost, *psqlDatabase, *psqlAppName, *psqlPassword;
  WSpinBox *psqlPort;
  postgresqlSettingsForm->add(psqlHost = new WLineEdit, "wizard.postgresql.host");
  postgresqlSettingsForm->add(psqlPort = new WSpinBox, "wizard.postgresql.port");
  postgresqlSettingsForm->add(psqlAppName = new WLineEdit, "wizard.postgresql.appname");
  postgresqlSettingsForm->add(psqlDatabase = new WLineEdit, "wizard.postgresql.database");
  postgresqlSettingsForm->add(psqlUser = new WLineEdit, "wizard.postgresql.username");
  postgresqlSettingsForm->add(psqlPassword = new WLineEdit, "wizard.postgresql.password");
  psqlPassword->setEchoMode(WLineEdit::Password);
  psqlPort->setRange(0, 65535);
  postgresqlSettings->addWidget(postgresqlSettingsForm);
  
  psqlHost->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Hostname, psqlHost->text().toUTF8());});
  psqlPort->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Port, psqlPort->value() );});
  psqlAppName->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Application, psqlAppName->text().toUTF8());});
  psqlDatabase->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Database, psqlDatabase->text().toUTF8());});
  psqlUser->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Username, psqlUser->text().toUTF8());});
  psqlPassword->changed().connect([=](_n1){Setting::write(Setting::PostgreSQL_Password, psqlPassword->text().toUTF8());});
  
  groupBox->addWidget(postgresqlSettings);
  buttonGroup->checkedChanged().connect([=](WRadioButton *b, _n5){
    Settings::databaseType(b == sqlite3 ? Settings::Sqlite3 : Settings::PostgreSQL);
    postgresqlSettings->setHidden(b != postgresql);
  });
#ifndef HAVE_POSTGRES
  postgresql->setEnabled(false);
#endif
  stack->addWidget(groupBox);
  showPage[DatabaseSetup] = [=] {
    displayPage(groupBox, FileSystemChooserPage, Authentication);
    auto dbType = Settings::databaseType();
    buttonGroup->setCheckedButton(dbType == Settings::Sqlite3 ? sqlite3 : postgresql);
    postgresqlSettings->setHidden(dbType != Settings::PostgreSQL);
    
    psqlHost->setText(Settings::postgresqlHost());
    psqlPort->setValue(Settings::postgresqlPort());
    psqlAppName->setText(Settings::postgresqlApplication());
    psqlDatabase->setText(Settings::postgresqlDatabase());
    psqlUser->setText(Settings::postgresqlUsername());
    psqlPassword->setText(Settings::postgresqlPassword());
  };
}


void PandoricaWizard::Private::addAuthentication()
{
  WGroupBox *groupBox = WW<WGroupBox>(WString::tr("wizard.authentication"));
  stack->addWidget(groupBox);
  WButtonGroup *buttonGroup = new WButtonGroup(groupBox);
  WRadioButton *authNone = addRadio("wizard.authentication.none", groupBox, buttonGroup, Settings::NoAuth);
  WRadioButton *authBasic = addRadio("wizard.authentication.simple", groupBox, buttonGroup, Settings::AuthenticateSimple);
  WRadioButton *authACL = addRadio("wizard.authentication.full", groupBox, buttonGroup, Settings::AuthenticateACL);
  WCheckBox *emailVerification = WW<WCheckBox>(WString::tr("wizard.authentication.emailverification")).setHidden(true);
  groupBox->addWidget(emailVerification);

  map<Settings::AuthenticationMode, WRadioButton*> buttons { {Settings::NoAuth, authNone}, {Settings::AuthenticateSimple, authBasic}, {Settings::AuthenticateACL, authACL} };
  
  buttonGroup->checkedChanged().connect([=](WRadioButton *b, _n5){
    auto authMode = static_cast<Settings::AuthenticationMode>(buttonGroup->id(b));
    emailVerification->setHidden(authMode == Settings::NoAuth);
    Settings::authenticationMode(authMode);
    emailVerification->setChecked(Settings::emailVerificationMandatory());
    displayPage(groupBox, DatabaseSetup, authMode == Settings::NoAuth ? CongratsPage : AdminUserCreation);
  });
  emailVerification->changed().connect([=](_n1){
    Setting::write(Setting::EmailVerificationMandatory, emailVerification->isChecked());
  });
  
  showPage[Authentication] = [=] {
    auto authenticationMode = Settings::authenticationMode();
    displayPage(groupBox, DatabaseSetup, authenticationMode == Settings::NoAuth ? CongratsPage : AdminUserCreation);
    buttonGroup->setCheckedButton(buttons.at(authenticationMode));
    emailVerification->setHidden(authenticationMode == Settings::NoAuth);
    emailVerification->setChecked(Settings::emailVerificationMandatory());
  };
}


void PandoricaWizard::Private::addAdminUserPage()
{

  WGroupBox *groupBox = WW<WGroupBox>(WString::tr("wizard.admincreation"));
  stack->addWidget(groupBox);
  showPage[AdminUserCreation] = [=] {
    groupBox->clear();
    groupBox->addWidget(new WText{WString::tr("wizard.admincreation.registrationhint")});
    Session *session = new Session(true);
    WContainerWidget *container = new WContainerWidget(groupBox);
    Auth::RegistrationModel *model = new Auth::RegistrationModel(session->auth(), session->users(), session->login());
    auto registrationWidget = new Auth::RegistrationWidget();
    registrationWidget->setModel(model);
    model->addPasswordAuth(&session->passwordAuth());
    session->users().setNewUserStatus(Auth::User::Normal);
    container->addWidget(registrationWidget);
    auto registrationWidgetDeleted = new WObjectScope([=]{
//       session->flush();
      if(model->login().loggedIn() && model->login().user().isValid()) {
        Dbo::Transaction t(*session);
        auto user = session->user();
        wApp->log("notice") << "Registered user";
        adminGroup(t).modify()->users.insert(user);
      }
      groupBox->clear();
      groupBox->addWidget(new WText{WString::tr("wizard.admincreation.registered")});
      WServer::instance()->post(wApp->sessionId(), [=]{
        delete model;
        delete session;
      });
    }, registrationWidget);
    displayPage(groupBox, Authentication, CongratsPage);
  };
}

void PandoricaWizard::Private::addFinishPage()
{
  WGroupBox *groupBox = WW<WGroupBox>(WString::tr("wizard.finished")).add(WW<WText>(WString::tr("wizard.finished.text")));
  stack->addWidget(groupBox);
  showPage[CongratsPage] = [=] {
    displayPage(groupBox, Settings::authenticationMode() == Settings::NoAuth ? Authentication : AdminUserCreation, None);
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


WRadioButton* PandoricaWizard::Private::addRadio(const string& textKey, WContainerWidget* container, WButtonGroup* buttonGroup, int id)
{
  WRadioButton *button = WW<WRadioButton>(WString::tr(textKey)).setInline(false);
  WLabel *label = WW<WLabel>(WString::tr(textKey + ".label")).setInline(false);
  WW<WContainerWidget>(container).add(button).add(label);
  label->setBuddy(button);
  buttonGroup->addButton(button, id);
  return button;
}



GroupPtr PandoricaWizard::Private::adminGroup(Dbo::Transaction &transaction)
{
  GroupPtr adminGroup = transaction.session().find<Group>().where("group_name = ?").bind("Admin");
  wApp->log("notice") << "Admin group found: " << adminGroup << (adminGroup ? string{": "} + adminGroup->groupName() : string{});
  if(! adminGroup) {
    wApp->log("notice") << "Creating main admin group";
    adminGroup = transaction.session().add(new Group("Admin", true));
  }
  return adminGroup;
}


PandoricaWizard::~PandoricaWizard()
{
  wApp->log("notice") << "Finalizing settings...";
  Session session(true);
  Dbo::Transaction t(session);
  auto adminGroup = d->adminGroup(t);
  
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
    WW<WContainerWidget>()
    .add(
      WW<WToolBar>().addCss("pull-right")
      .addButton(d->previous = WW<WPushButton>(WString::tr("button.previous")).css("btn-warning").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->previousPage](); }))
      .addButton(d->next = WW<WPushButton>(WString::tr("button.next")).css("btn-primary").setEnabled(false).onClick([=](WMouseEvent){ d->showPage[d->nextPage](); }))
      .addButton(d->finish = WW<WPushButton>(WString::tr("button.finish")).css("btn-success").setEnabled(false).onClick([=](WMouseEvent){ delete this; }))
    )
    .add(new WBreak)
    .add(d->stack)
  );
  d->addBeginPage();
  d->addPandoricaModePage();
  d->addFileSystemChooser();
  d->addDatabaseSetup();
  d->addAuthentication();
  d->addAdminUserPage();
  d->addFinishPage();
  d->showPage[Private::Begin]();
}
