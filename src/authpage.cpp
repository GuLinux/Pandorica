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

using namespace std;
using namespace Wt;
using namespace Wt::Auth;
using namespace WtCommons;

namespace dbo = Wt::Dbo;

AuthWidgetCustom::AuthWidgetCustom(const AuthService& baseAuth, AbstractUserDatabase& users, Login& login, WContainerWidget* parent)
  : AuthWidget(baseAuth, users, login, parent)
{
}

RegistrationModel* AuthWidgetCustom::createRegistrationModel()
{
  RegistrationModel *model = AuthWidget::createRegistrationModel();
  model->setEmailPolicy(Settings::emailVerificationMandatory() ? RegistrationModel::EmailMandatory : RegistrationModel::EmailOptional);
  return model;
}

AuthPage::Private::Private(Session* session, AuthPage* q) : session(session), q(q)
{
}


void AuthWidgetCustom::createOAuthLoginView()
{
  Wt::Auth::AuthWidget::createOAuthLoginView();
  if(!resolveWidget("icons")) return;
  WContainerWidget *icons = (WContainerWidget*) resolveWidget("icons");
  for(auto child: icons->children()) {
    WImage *image = dynamic_cast<WImage*>(child);
    if(!image) continue;
    string imageUrl = image->imageLink().url();
    
    boost::replace_all(imageUrl, "css", Settings::staticDeployPath() + "/icons/oauth" );
    log("notice") << "found oauth image: " << image->imageLink().url() << " >> " << imageUrl;
    image->setImageLink(imageUrl);
  }
}


Message::Message(Wt::WString text, Wt::WContainerWidget* parent): WTemplate(parent)
{
  addStyleClass("alert");
  setTemplateText(WString("<button type=\"button\" class=\"close\" data-dismiss=\"alert\">&times;</button>{1}").arg(text), Wt::XHTMLUnsafeText);
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
  Scope scope([=]{ loginChanged.emit(session->login().state()); });
  if(!session->login().loggedIn()) {
//     log("notice") << __PRETTY_FUNCTION__ << ": logout detected";
//     loggedOut.emit();
//     messagesContainer->clear();
//     q->doJavaScript(animationJs(q, "fadeIn"));
    return;
  }
  log("notice") << "User logged in";
  dbo::Transaction t(*session);
  //   changeSessionId();
  Auth::User user = session->login().user();
  WPushButton *refreshButton = WW<WPushButton>(wtr("button.retry")).css("btn btn-link").onClick([this](WMouseEvent) {
    authEvent();
  }).setAttribute("data-dismiss", "alert");
  if(Settings::emailVerificationMandatory() && !user.unverifiedEmail().empty()) {
    log("notice") << "User email empty, unconfirmed?";
    Message *message = WW<Message>(wtr("user.need_mail_verification")).addCss("alert-block");
    message->bindWidget("refresh", refreshButton);
    messagesContainer->addWidget(message);
    return;
  }
  log("notice") << "User email confirmed, or verification not mandatory";

  if(seedIfNoAdmins(t, user)) return;
  
  if(session->user()->groups.size() <= 0) {
    Message *message = WW<Message>(wtr("user.need_to_be_enabled")).addCss("alert-block");
    if(!mailSent) {
      ::Utils::mailForUnauthorizedUser(user.email(), user.identity(Auth::Identity::LoginName));
      mailSent = true;
    }
    messagesContainer->addWidget(message);
    message->bindWidget("refresh", refreshButton);
    return;
  }
//   q->addStyleClass("hidden");
  q->doJavaScript(animationJs(q, "fadeOut"));
//   loggedIn.emit();
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
      authEvent();
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
  
  addWidget(WW<WText>(WString("<h1 style=\"text-align: center;\">{1}</h1>").arg(wtr("site-title"))));
  d->authWidget = new Wt::Auth::AuthWidget(Session::auth(), session->users(), session->login());
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
