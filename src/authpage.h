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

#ifndef AUTHPAGE_H
#define AUTHPAGE_H

#include <Wt/WContainerWidget>
#include <Wt/Dbo/Transaction>

class Session;
namespace StreamingPrivate {
  class AuthPagePrivate;
}
class AuthPage : public Wt::WContainerWidget
{
public:
    ~AuthPage();
    AuthPage(Session *session, Wt::WContainerWidget* parent = 0);
  Wt::Signal<> &loggedIn() const;
  Wt::Signal<> &loggedOut() const;
  void initAuth();
private:
    StreamingPrivate::AuthPagePrivate* const d;
};

#endif // AUTHPAGE_H