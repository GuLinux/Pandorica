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
    : _filename(path.filename().string()), _filepath(path.string()), _playStarted(Wt::WDateTime::currentDateTime().toPosixTime()) {}
  ~SessionDetails() {}
  
  std::string filename() const { return _filename; }
  std::string filepath() const { return _filepath; }
  void ended() { if(_playEnded == boost::posix_time::not_a_date_time) _playEnded = Wt::WDateTime::currentDateTime().toPosixTime(); }
  Wt::WDateTime playStarted() const { return Wt::WDateTime::fromPosixTime(_playStarted); }
  Wt::WDateTime playEnded() const { return Wt::WDateTime::fromPosixTime(_playEnded); }
  
private:
  std::string _filename;
  std::string _filepath;
  boost::posix_time::ptime _playStarted;
  boost::posix_time::ptime _playEnded;
  Wt::Dbo::ptr<SessionInfo> _sessionInfo;
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
