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


#ifndef GROUP_H
#define GROUP_H

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>

class User;
class Group;
namespace dbo = Wt::Dbo;

class GroupPath {
public:
  GroupPath() = default;
  GroupPath(std::string path) : _path(path) {}
  template<class Action>
  void persist(Action& a) {
    dbo::field(a, _path, "path");
    dbo::belongsTo(a, _group, "group");
  }
  inline std::string path() const { return _path; }
private:
  std::string _path;
  dbo::ptr<Group> _group;
};

typedef dbo::ptr<GroupPath> GroupPathPtr;

class Group
{
public:
  Group() = default;
  Group(std::string groupName, bool isAdmin = false);
  template<class Action>
  void persist(Action& a) {
    dbo::hasMany(a, _users, dbo::ManyToMany, "groups_users");
    dbo::field(a, _groupName, "group_name");
    dbo::field(a, _isAdmin, "is_admin");
    dbo::hasMany(a, _groupPaths, dbo::ManyToOne, "group");
  }
  inline std::string groupName() const { return _groupName; }
  inline void setGroupName(const std::string &groupName) { _groupName = groupName; }
  inline bool isAdmin() const { return _isAdmin; }
  std::list< std::string > allowedPaths() const;
  dbo::collection< dbo::ptr<User> > users() const { return _users; }
  dbo::collection< dbo::ptr<GroupPath> > groupPaths() const { return _groupPaths; }

  void addUser(const dbo::ptr<User> &user, dbo::Transaction &transaction);
  void removeUser(const dbo::ptr<User> &user, dbo::Transaction &transaction);

  void addPath(const GroupPathPtr &path);
  void removePath(const GroupPathPtr &path);

private:
  dbo::collection< dbo::ptr<User> > _users;
  dbo::collection< dbo::ptr<GroupPath> > _groupPaths;
  std::string _groupName;
  bool _isAdmin;
};

typedef dbo::ptr<Group> GroupPtr;

#endif // GROUP_H
