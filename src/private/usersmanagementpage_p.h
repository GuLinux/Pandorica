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
#include <functional>
class Group;
namespace Wt
{
  class WTable;
  class WPushButton;
}

class Session;
class UsersManagementPage::Private
{
  public:
    Private( UsersManagementPage *q, Session *session );
    virtual ~Private();

    Session *session;
    Wt::WTable *invitedUsersContainer;
    Wt::WTable *usersContainer;
    void populate();
    void addUserRow( Wt::Dbo::ptr< AuthInfo > &authInfo, Wt::Dbo::ptr< User > &user, Wt::WTable *table, Wt::Dbo::Transaction &transaction );
    typedef std::function<void(const Wt::Dbo::ptr<Group>&, Wt::Dbo::Transaction &)> GroupModTrigger;
    typedef std::function<bool(const Wt::Dbo::ptr<Group>&)> GroupSelection;
    Wt::WPushButton *groupsButton(Wt::Dbo::Transaction &transaction, GroupSelection groupSelection, GroupModTrigger onGroupChecked, GroupModTrigger onGroupUnchecked);
    void invite(std::string email, const std::vector<Wt::Dbo::ptr<Group>> &groups, bool sendEmail);
  private:
    class UsersManagementPage *const q;
};



#endif // PRIVATE_H
