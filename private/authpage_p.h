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

#ifndef AUTHPAGEPRIVATE_H
#define AUTHPAGEPRIVATE_H
#include <Wt/Auth/AuthWidget>

class Session;
class AuthPage;
namespace StreamingPrivate {
class AuthWidgetCustom;
class AuthPagePrivate
{
public:
    AuthPagePrivate(Session *session, AuthPage* q);
    virtual ~AuthPagePrivate();
    Wt::WContainerWidget* messagesContainer;
    Wt::Signal<> loggedIn;
    Wt::Signal<> loggedOut;
    Session *session;
    bool mailSent = false;
    AuthWidgetCustom* authWidget;
    void authEvent();
private:
    class AuthPage* const q;
    bool seedIfNoAdmins(Wt::Dbo::Transaction& transaction, Wt::Auth::User& user);
};

class AuthWidgetCustom : public Wt::Auth::AuthWidget {
public:
    AuthWidgetCustom(const Wt::Auth::AuthService& baseAuth, Wt::Auth::AbstractUserDatabase& users, Wt::Auth::Login& login, Wt::WContainerWidget* parent = 0);
protected:
    virtual Wt::Auth::RegistrationModel* createRegistrationModel();
    virtual void createOAuthLoginView();
};


class Message : public Wt::WTemplate {
public:
    Message(Wt::WString text, Wt::WContainerWidget* parent = 0);
};


}


#endif // STREAMINGPRIVATE::AUTHPAGEPRIVATE_H
