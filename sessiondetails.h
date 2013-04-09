#ifndef SESSIONDETAILS_H_
#define SESSIONDETAILS_H_

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WGlobal>
#include <Wt/WDateTime>
#include <boost/filesystem/path.hpp>

class SessionInfo;
class SessionDetails {
public:
  SessionDetails() {}
  SessionDetails(boost::filesystem::path path)
    : _filename(path.filename().string()), _filepath(path.string()), _playStarted(Wt::WDateTime::currentDateTime().toTime_t()) {}
  ~SessionDetails() {}
  
  std::string filename() const { return _filename; }
  std::string filepath() const { return _filepath; }
  void ended() { if(!_playEnded) _playEnded = Wt::WDateTime::currentDateTime().toTime_t(); }
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
    Wt::Dbo::field(a, _playStarted, "play_started");
    Wt::Dbo::field(a, _playEnded, "play_ended");
    Wt::Dbo::belongsTo(a, _sessionInfo, "session_info");
  }
};

typedef Wt::Dbo::ptr<SessionDetails> SessionDetailsPtr;


#endif // SESSIONDETAILS_H_
