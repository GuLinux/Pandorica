#ifndef SESSIONDETAILS_H_
#define SESSIONDETAILS_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include "authorizeduser.h"

class SessionInfo;
class SessionDetails {
public:
  SessionDetails() {}
  ~SessionDetails() {}
  
  std::string filename() const { return _filename; }
  std::string filepath() const { return _filepath; }
  Wt::WDateTime playStarted() const { return Wt::WDateTime::fromTime_t(_playStarted); }
  Wt::WDateTime playEnded() const { return Wt::WDateTime::fromTime_t(_playEnded); }
  
private:
  std::string _filename;
  std::string _filepath;
  long _playStarted = 0;
  long _playEnded = 0;
  dbo::ptr<SessionInfo> _sessionInfo;
public:
    template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, _filename, "filename");
    Wt::Dbo::field(a, _filepath, "filepath");
    Wt::Dbo::field(a, _playStarted, "playStarted");
    Wt::Dbo::field(a, _playEnded, "playEnded");
    Wt::Dbo::belongsTo(a, _sessionInfo, "sessionInfo");
  }
};

typedef Wt::Dbo::ptr<SessionDetails> SessionDetailsPtr;


#endif // SESSIONDETAILS_H_
