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




#include "authpage.h"
#include "private/authpage_p.h"
#include <Wt/Auth/RegistrationModel>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WLineEdit>
#include <Wt/WLabel>
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include "utils/utils.h"

#include <Wt/Dbo/Transaction>
#include <Wt/WTimer>
#include <Wt/WConfig.h>
#include <Wt/WImage>

#include "Models/models.h"
#include "settings.h"
#include <boost/format.hpp>
#include "utils/d_ptr_implementation.h"
#include "pandoricawizard.h"

using namespace std;
using namespace Wt;
using namespace Auth;
using namespace WtCommons;

namespace dbo = Wt::Dbo;

AuthPage::Private::Private(Session* session, AuthPage* q) : session(session), q(q)
{
}


CustomAuthWidget::CustomAuthWidget(const Auth::AuthService& baseAuth, Auth::AbstractUserDatabase& users, Auth::Login& login, WStackedWidget *stack, WContainerWidget* parent)
  : Auth::AuthWidget(baseAuth, users, login, parent), stack(stack)
{

}

void CustomAuthWidget::registerNewUser(const Wt::Auth::Identity& oauth)
{
//     Wt::Auth::AuthWidget::registerNewUser(oauth);
  auto registrationWidget = createRegistrationView(oauth);
  stack->addWidget(registrationWidget);
  stack->setCurrentWidget(registrationWidget);
}


void CustomAuthWidget::recreateView()
{
  createLoginView();
}




Message::Message(WString text, WContainerWidget* parent): WTemplate(parent)
{
  addStyleClass("alert");
  setTemplateText(WString("<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>{1}").arg(text), XHTMLUnsafeText);
}

string animationJs(WWidget *widget, string animation) {
  return (boost::format("$('#%s').%s();") % widget->id() % animation).str();
}

void AuthPage::Private::authEvent() {
  static map<Auth::LoginState, string> states {
    {Auth::LoggedOut, "LoggedOut"},
    {Auth::DisabledLogin, "DisabledLogin"},
    {Auth::WeakLogin, "WeakLogin"},
    {Auth::StrongLogin, "StrongLogin"},
  };
  log("notice") << __PRETTY_FUNCTION__ << ": state=" << states[session->login().state()];
  if(session->login().state() == Auth::DisabledLogin) {
    log("notice") << "Got disabled login";
//     session->login().logout();
    authWidget->recreateView();
    return;
  }
  Scope scope([=]{ loginChanged.emit(session->login().state()); });
  if(!session->login().loggedIn() || session->login().state() == Auth::DisabledLogin) {
    return;
  }
  log("notice") << "User logged in";
  dbo::Transaction t(*session);
  //   changeSessionId();
  Auth::User user = session->login().user();
  q->doJavaScript(animationJs(q, "fadeOut"));
  seedIfNoAdmins(t, user);
}

bool AuthPage::Private::seedIfNoAdmins(dbo::Transaction& transaction, Auth::User &user)
{
  dbo::collection<GroupPtr> adminGroups = session->find<Group>().where("is_admin = ?").bind(true);
  int adminUsersCount = 0;
  for(auto group: adminGroups) {
    adminUsersCount += group->users.size();
  }
  wApp->log("notice") << "adminUsersCount: " << adminUsersCount << ", admin groups: " << adminGroups.size();
  if(adminGroups.size() == 0 || adminUsersCount == 0) {
    // transaction.rollback();
    WDialog *addMyselfToAdmins = new WDialog{wtr("admin_missing_dialog_title")};
    addMyselfToAdmins->contents()->addWidget(new WText{wtr("admin_missing_dialog_text").arg(user.identity("loginname")) });
    WLineEdit *groupName = new WLineEdit;
    groupName->setText(wtr("admin_missing_dialog_default_groupname"));
    auto groupNameLabel = new WLabel(wtr("admin_missing_dialog_groupname_label"));
    groupNameLabel->setBuddy(groupName);
    WContainerWidget *formInline = WW<WContainerWidget>().css("form-inline").add(groupNameLabel).add(groupName).padding(5);
    addMyselfToAdmins->contents()->addWidget(formInline);
    addMyselfToAdmins->footer()->addWidget(WW<WPushButton>(wtr("button.cancel")).onClick([=](WMouseEvent){ addMyselfToAdmins->reject(); }).css("btn btn-danger"));
    addMyselfToAdmins->footer()->addWidget(WW<WPushButton>(wtr("button.ok")).onClick([=](WMouseEvent){
      dbo::Transaction t(*session);
      GroupPtr newGroup = session->add(new Group{groupName->text().toUTF8(), true});
      auto sessionUser = session->user();
      newGroup.modify()->users.insert(sessionUser);
      t.commit();
      ::Utils::mailForNewAdmin(user.email(), user.identity("loginname"));
      addMyselfToAdmins->accept();
    }).css("btn btn-primary"));
    addMyselfToAdmins->show();
    return true;
  }
  return false;
}


AuthPage::~AuthPage()
{
}

Signal<Auth::LoginState>& AuthPage::loginChanged() const
{
  return d->loginChanged;
}




AuthPage::AuthPage(Session* session, WContainerWidget* parent)
    : WContainerWidget(parent), d(session, this)
{
  session->login().changed().connect([=](_n6){ d->authEvent(); });
  d->stack = new WStackedWidget;
  addStyleClass("container");
  addWidget(WW<WText>(WString("<h1 style=\"text-align: center;\">{1}</h1>").arg(wtr("site-title"))));
  addWidget(d->stack);
  if(! Setting::value<bool>(Setting::PandoricaSetup, false)) {
    auto wizard = new PandoricaWizard;
    d->stack->addWidget(wizard);
    d->stack->setCurrentWidget(wizard);
    wizard->finished().connect(d.get(), &AuthPage::Private::setupLogin);
  }
  else
    d->setupLogin();
  addWidget(d->messagesContainer = new WContainerWidget());
}

void AuthPage::Private::setupLogin()
{
  if(Settings::pandoricaMode() == Settings::Simple) {
    session->login().login(session->users().findWithIdentity("pandorica", "Admin"));
    wApp->log("notice") << "Simple login mode found: logging in admin user: " << session->login().loggedIn();
    wApp->changeSessionId();
    return;
  }
  authWidget = new CustomAuthWidget(Session::auth(), session->users(), session->login(), stack);
  authWidget->model()->addPasswordAuth(&Session::passwordAuth());
  authWidget->model()->addOAuth(Session::oAuth());
  authWidget->setRegistrationEnabled(true);
  stack->addWidget(authWidget);
  authWidget->processEnvironment();
}


void AuthPage::initAuth()
{
}
