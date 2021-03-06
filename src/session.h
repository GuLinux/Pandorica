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



#ifndef SESSION_H_
#define SESSION_H_

#include <Wt/Auth/Login>

#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/SqlConnection>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/AuthService>
#include "Models/models.h"

#include "utils/d_ptr.h"

namespace dbo = Wt::Dbo;

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;
class Session : public dbo::Session
{
  public:
    class WriteLock;
    std::shared_ptr<WriteLock> writeLock() const;
    static void configureAuth();

    Session( bool full = false );
    ~Session();

    dbo::ptr<User> user();
    dbo::ptr<User> user(const Wt::Auth::User& authUser);

    UserDatabase &users();
    Wt::Auth::Login &login();
    Wt::Dbo::SqlConnection *connection() const;

    static const Wt::Auth::AuthService &auth();
    static const Wt::Auth::PasswordService &passwordAuth();
    static const std::vector<const Wt::Auth::OAuthService *> &oAuth();
  private:
    D_PTR;
};

#endif // SESSION_H_
