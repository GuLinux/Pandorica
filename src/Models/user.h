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



#ifndef USER_H_
#define USER_H_

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>

class Media;
class MediaRating;
class Group;
class SessionInfo;
class Comment;
namespace dbo = Wt::Dbo;

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User {
public:
  /* You probably want to add other user information here */

  template<class Action>
  void persist(Action& a)
  {
    dbo::hasMany(a, sessionInfos, dbo::ManyToOne, "user");
    dbo::hasMany(a, comments, dbo::ManyToOne, "user");
    dbo::hasMany(a, groups, dbo::ManyToMany, "groups_users");
    dbo::hasMany(a, ratings, dbo::ManyToOne, "user");
  }
  
  static void rate(dbo::ptr<User> userPtr, const Media &media, int rating, Wt::Dbo::Transaction &transaction);
  
  bool isAdmin() const;
  std::list<std::string> allowedPaths() const;
  
  dbo::collection<dbo::ptr<SessionInfo>> sessionInfos;
  dbo::collection<dbo::ptr<Comment>> comments;
  dbo::collection< dbo::ptr<Group>> groups;
  dbo::collection<dbo::ptr<MediaRating>> ratings;
};

typedef dbo::ptr<User> UserPtr;

#endif // USER_H_
