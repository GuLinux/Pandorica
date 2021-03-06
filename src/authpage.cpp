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
#include <Wt/Auth/RegistrationWidget>

#include "Models/models.h"
#include "settings.h"
#include <boost/format.hpp>
#include "utils/d_ptr_implementation.h"
#include "pandoricawizard.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

namespace dbo = Wt::Dbo;

AuthPage::Private::Private(Session* session, AuthPage* q) : session(session), q(q)
{
}

class CustomRegistrationWidget : public Auth::RegistrationWidget {
public:
  CustomRegistrationWidget(Session *session, Wt::Auth::AuthWidget *p) : Auth::RegistrationWidget(p), session(session) {}
protected:
  virtual void registerUserDetails (Auth::User &authUser);
private:
  Session *session;
};

void CustomRegistrationWidget::registerUserDetails(Auth::User &authUser) {
  dbo::Transaction t(*session);
  auto authInfo = session->users().find(authUser);
  if(! authInfo)
    return;
  dbo::ptr<User> user = authInfo->user();
  dbo::ptr<User> invitedUser = session->find<User>().where("invited_email_address = ?").bind(authInfo->email().empty() ? authInfo->unverifiedEmail() : authInfo->email()); // TODO: move inside authPage
  
  if(!user) {
    user = invitedUser ? invitedUser : session->add(new User);
    user.modify()->invitedEmailAddress.reset();
    authInfo.modify()->setUser(user);
    authInfo.flush();
    session->flush();
  }
    wApp->log("notice") << "automatically created user: " << user.id();
}



Wt::WWidget* CustomAuthWidget::createRegistrationView(const Wt::Auth::Identity& id)
{
  //return Wt::Auth::AuthWidget::createRegistrationView(id);
  
  CustomRegistrationWidget *w = new CustomRegistrationWidget(session, this);
  Wt::Auth::RegistrationModel *model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);

  w->setModel(model);
  return w;
}


CustomAuthWidget::CustomAuthWidget(const Auth::AuthService& baseAuth, Auth::AbstractUserDatabase& users, Auth::Login& login, WStackedWidget* stack, Session* session, WContainerWidget* parent)
  : Auth::AuthWidget(baseAuth, users, login, parent), stack(stack), session(session)
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

  WServer::instance()->post(wApp->sessionId(), [=]{d->setupLogin();});
  addWidget(d->messagesContainer = new WContainerWidget());
}

void AuthPage::Private::setupLogin()
{
  if(Settings::authenticationMode() == Settings::Settings::NoAuth) {
    session->login().login(session->users().findWithIdentity("pandorica", "Admin"));
    wApp->log("notice") << "Simple login mode found: logging in admin user: " << session->login().loggedIn();
    return;
  }
  authWidget = new CustomAuthWidget(Session::auth(), session->users(), session->login(), stack, session);
  authWidget->model()->addPasswordAuth(&Session::passwordAuth());
  authWidget->model()->addOAuth(Session::oAuth());
  authWidget->setRegistrationEnabled(true);
  stack->addWidget(authWidget);
  authWidget->processEnvironment();
}


void AuthPage::initAuth()
{
}
