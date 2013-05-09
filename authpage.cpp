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

#include "authpage.h"
#include "private/authpage_p.h"
#include <Wt/Auth/RegistrationModel>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WLineEdit>
#include <Wt/WLabel>
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include "utils.h"
#include <Wt/Dbo/Transaction>
#include <Wt/WTimer>

#include "group.h"
#include "user.h"
#include "comment.h"
#include "sessioninfo.h"
#include "sessiondetails.h"

using namespace StreamingPrivate;
using namespace std;
using namespace Wt;
using namespace Wt::Auth;

namespace dbo = Wt::Dbo;

AuthWidgetCustom::AuthWidgetCustom(const AuthService& baseAuth, AbstractUserDatabase& users, Login& login, WContainerWidget* parent)
  : AuthWidget(baseAuth, users, login, parent)
{
}

RegistrationModel* AuthWidgetCustom::createRegistrationModel()
{
  RegistrationModel *model = AuthWidget::createRegistrationModel();
  model->setEmailPolicy(RegistrationModel::EmailMandatory);
  return model;
}

AuthPagePrivate::AuthPagePrivate(Session* session, AuthPage* q) : session(session), q(q)
{
}

AuthPagePrivate::~AuthPagePrivate()
{
}


Message::Message(Wt::WString text, Wt::WContainerWidget* parent): WTemplate(parent)
{
  addStyleClass("alert");
  setTemplateText(WString("<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>{1}").arg(text), Wt::XHTMLUnsafeText);
}

void AuthPagePrivate::authEvent() {
  if(!session->login().loggedIn()) {
    loggedOut.emit();
    messagesContainer->clear();
    q->setStyleClass("");
    return;
  }
  log("notice") << "User logged in";
  dbo::Transaction t(*session);
  //   changeSessionId();
  Auth::User user = session->login().user();
  WPushButton *refreshButton = WW<WPushButton>("Retry").css("btn btn-link").onClick([this](WMouseEvent) {
    authEvent();
  }).setAttribute("data-dismiss", "alert");
  if(user.email().empty()) {
    log("notice") << "User email empty, unconfirmed?";
    Message *message = WW<Message>("You need to verify your email address before logging in.<br />\
    Please check your inbox.<br />${refresh}").addCss("alert-block");
    message->bindWidget("refresh", refreshButton);
    messagesContainer->addWidget(message);
    return;
  }
  log("notice") << "User email confirmed";
  dbo::collection<GroupPtr> adminGroups = session->find<Group>().where("is_admin = ?").bind(true);
  int adminUsersCount = 0;
  for(auto group: adminGroups) {
    adminUsersCount += group->users.size();
  }
  wApp->log("notice") << "adminUsersCount: " << adminUsersCount << ", admin groups: " << adminGroups.size();
  if(adminGroups.size() == 0 || adminUsersCount == 0) {
    t.rollback();
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
      newGroup.modify()->users.insert(session->user());
      t.commit();
      ::Utils::mailForNewAdmin(user.email(), user.identity("loginname"));
      addMyselfToAdmins->accept();
      authEvent();
    }).css("btn btn-primary"));
    addMyselfToAdmins->show();
    return;
  }
  if(session->user()->groups.size() <= 0) {
    Message *message = WW<Message>("Your user is not yet authorized for viewing videos.<br />\
    The administrator should already have received an email and will add you when possible.<br />${refresh}").addCss("alert-block");
    if(!mailSent) {
      ::Utils::mailForUnauthorizedUser(user.email(), user.identity(Auth::Identity::LoginName));
      mailSent = true;
    }
    messagesContainer->addWidget(message);
    message->bindWidget("refresh", refreshButton);
    return;
  }
  q->setStyleClass("hidden"); // workaround: for wt 3.3.x hide() doesn't seem to work...
  loggedIn.emit();
}

AuthPage::~AuthPage()
{
    delete d;

}

Signal<>& AuthPage::loggedIn() const
{
  return d->loggedIn;
}

Signal<>& AuthPage::loggedOut() const
{
  return d->loggedOut;
}



AuthPage::AuthPage(Session* session, WContainerWidget* parent)
    : d(new AuthPagePrivate(session, this))
{
  session->login().changed().connect(d, &AuthPagePrivate::authEvent);
  
  addWidget(WW<WText>(WString("<h1 style=\"text-align: center;\">{1}</h1>").arg(wtr("site-title"))));
  d->authWidget = new AuthWidgetCustom(Session::auth(), session->users(), session->login());
  d->authWidget->model()->addPasswordAuth(&Session::passwordAuth());
  d->authWidget->model()->addOAuth(Session::oAuth());
  d->authWidget->setRegistrationEnabled(true);
  addWidget(d->authWidget);
  addWidget(d->messagesContainer = new WContainerWidget());
}

void AuthPage::initAuth()
{
  d->authWidget->processEnvironment();
}
