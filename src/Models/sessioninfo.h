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


#ifndef SESSIONINFO_H_
#define SESSIONINFO_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>

class User;
class SessionDetails;
class SessionInfo {
public:
  SessionInfo() {}
  SessionInfo(Wt::Dbo::ptr<User> user, std::string sessionId, std::string ip)
    : _sessionId(sessionId), _ip(ip), _sessionStarted(Wt::WDateTime::currentDateTime().toTime_t()), _user(user) {}
  ~SessionInfo() {}
  
  std::string sessionId() const { return _sessionId; }
  std::string ip() const { return _ip; }
  Wt::WDateTime sessionStarted() const { return Wt::WDateTime::fromTime_t(_sessionStarted); }
  Wt::WDateTime sessionEnded() const { return Wt::WDateTime::fromTime_t(_sessionEnded); }
  Wt::Dbo::collection<Wt::Dbo::ptr<SessionDetails>> sessionDetails() { return _sessionDetails; }
  void end() { _sessionEnded = Wt::WDateTime::currentDateTime().toTime_t(); }
  
private:
  std::string _sessionId;
  std::string _ip;
  long _sessionStarted = 0;
  long _sessionEnded = 0;
  Wt::Dbo::ptr<User> _user;
  Wt::Dbo::collection<Wt::Dbo::ptr<SessionDetails>> _sessionDetails;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::id(a, _sessionId, "session_id", 16);
    Wt::Dbo::field(a, _ip, "ip");
    Wt::Dbo::field(a, _sessionStarted, "session_started");
    Wt::Dbo::field(a, _sessionEnded, "session_ended");
    Wt::Dbo::hasMany(a, _sessionDetails, Wt::Dbo::ManyToOne, "session_info");
    Wt::Dbo::belongsTo(a, _user, "user");
  }
};

typedef Wt::Dbo::ptr<SessionInfo> SessionInfoPtr;

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<SessionInfo>
    {
      typedef std::string IdType;
      static IdType invalidId() { return {}; }
      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };
  }
}

#endif // SESSIONINFO_H_
