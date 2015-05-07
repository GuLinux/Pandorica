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


#include "Models/models.h"

using namespace std;

Group::Group(std::string groupName, bool isAdmin)
: _groupName(groupName), _isAdmin(isAdmin)
{
}


list<string> Group::allowedPaths() const
{
  if(isAdmin())
    return {"/"};
  list<string> paths;
  for(auto path: _groupPaths) {
    paths.push_back(path->path());
  }
  return paths;
}

void Group::addUser(const dbo::ptr<User> &user, Wt::Dbo::Transaction &transaction) {
    _users.insert(user);
}

void Group::removeUser(const dbo::ptr<User> &user, Wt::Dbo::Transaction &transaction)
{
    _users.erase(user);
}

void Group::addPath(const GroupPathPtr& path)
{
  _groupPaths.insert(path);
}

void Group::removePath(const GroupPathPtr& path)
{
  _groupPaths.erase(path);
}
