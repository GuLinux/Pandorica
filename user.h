// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Auth/User>
#include <Wt/Dbo/Types>
#include <Wt/WGlobal>

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
    dbo::hasMany(a, _sessionInfos, dbo::ManyToOne, "user");
    dbo::hasMany(a, _comments, dbo::ManyToOne, "user");
    dbo::hasMany(a, _groups, dbo::ManyToMany, "groups_users");
  }
private:
  dbo::collection<dbo::ptr<SessionInfo>> _sessionInfos;
  dbo::collection<dbo::ptr<Comment>> _comments;
  dbo::collection< dbo::ptr<Group> > _groups;
};


#endif // USER_H_
