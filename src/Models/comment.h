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


#ifndef COMMENT_H_
#define COMMENT_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include <Wt/Auth/User>

class User;

class Comment {
public:
  Comment() {}
  Comment(std::string mediaId, Wt::Dbo::ptr<User> user, std::string content)
    : _mediaId(mediaId), _user(user), _content(content), _lastUpdated(Wt::WDateTime::currentDateTime().toTime_t()) {}
  ~Comment() {}
  std::string content() const { return _content; }
  std::string mediaId() const { return _mediaId; }
  Wt::WDateTime lastUpdated() const { return Wt::WDateTime::fromTime_t(_lastUpdated); }
  Wt::Dbo::ptr<User> user() const { return _user; }
private:
  std::string _content;
  std::string _mediaId;
  long _lastUpdated;
  Wt::Dbo::ptr<User> _user;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, _content, "content");
    Wt::Dbo::field(a, _mediaId, "media_id");
    Wt::Dbo::field(a, _lastUpdated, "last_updated");
    Wt::Dbo::belongsTo(a, _user, "user");
  }
};

typedef Wt::Dbo::ptr<Comment> CommentPtr;


#endif // COMMENT_H_
