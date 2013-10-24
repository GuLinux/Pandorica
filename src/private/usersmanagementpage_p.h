/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#ifndef PRIVATE_H
#define PRIVATE_H

#include "usersmanagementpage.h"
#include <Models/user.h>

namespace Wt
{
  class WTable;
}

class Session;
class UsersManagementPage::Private
{
  public:
    Private( UsersManagementPage *q, Session *session );
    virtual ~Private();

    Session *session;
    Wt::WTable *usersContainer;
    void populate();
    void addUserRow(const Wt::Dbo::ptr<AuthInfo> &user, Wt::Dbo::Transaction &transaction);
  private:
    class UsersManagementPage *const q;
};

#endif // PRIVATE_H
