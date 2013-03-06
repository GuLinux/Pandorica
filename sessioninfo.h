#ifndef SESSIONINFO_H_
#define SESSIONINFO_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include "authorizeduser.h"

class User;
class SessionDetails;
class SessionInfo {
public:
  SessionInfo() {}
  SessionInfo(Wt::Dbo::ptr<User> user, std::string sessionId, std::string ip, std::string email, std::string username, AuthorizedUser::Role role)
    : _sessionId(sessionId), _ip(ip), _email(email), _username(username), _role(role),
    _sessionStarted(Wt::WDateTime::currentDateTime().toTime_t()), _user(user) {}
  ~SessionInfo() {}
  
  std::string sessionId() const { return _sessionId; }
  std::string ip() const { return _ip; }
  std::string email() const { return _email; }
  std::string username() const { return _username; }
  AuthorizedUser::Role role() const { return _role; }
  std::string watching() const { return _watching; }
  Wt::WDateTime sessionStarted() const { return Wt::WDateTime::fromTime_t(_sessionStarted); }
  Wt::WDateTime sessionEnded() const { return Wt::WDateTime::fromTime_t(_sessionEnded); }
  Wt::Dbo::collection<Wt::Dbo::ptr<SessionDetails>> sessionDetails() { return _sessionDetails; }
  void setWatching(std::string watching) { this->_watching = watching; }
  void end() { _sessionEnded = Wt::WDateTime::currentDateTime().toTime_t(); }
  
private:
  std::string _sessionId;
  std::string _ip;
  std::string _email;
  std::string _username;
  AuthorizedUser::Role _role;
  std::string _watching;
  long _sessionStarted = 0;
  long _sessionEnded = 0;
  Wt::Dbo::ptr<User> _user;
  Wt::Dbo::collection<Wt::Dbo::ptr<SessionDetails>> _sessionDetails;
public:
    template<class Action>
  void persist(Action& a)
  {
    // TODO togliere username, email,forse anche Role usando AuthorizedUser e User
    Wt::Dbo::id(a, _sessionId, "session_id");
    Wt::Dbo::field(a, _username, "username");
    Wt::Dbo::field(a, _ip, "ip");
    Wt::Dbo::field(a, _email, "email");
    Wt::Dbo::field(a, _role, "role");
    Wt::Dbo::field(a, _watching, "watching");
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
      static IdType invalidId() { return std::string(); }
      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };
  }
}

#endif // SESSIONINFO_H_
