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


#ifndef MEDIACOLLECTIONPRIVATE_H
#define MEDIACOLLECTIONPRIVATE_H
#include <boost/filesystem.hpp>
#include <Wt/WSignal>
#include "media/media.h"
#include "media/mediacollection.h"

class Settings;
namespace Wt {
class WApplication;
}

class Session;
class MediaCollection::Private {
public:
  Private(Settings *settings, Session *session, Wt::WApplication *app) : settings(settings), session(session), app(app) {}
  void listDirectory(boost::filesystem::path path);
  bool isAllowed(boost::filesystem::path path);
public:
  Wt::WLoadingIndicator *loadingIndicator;
  Settings *settings;
  std::map<std::string,Media> collection;
  Wt::Signal<> scanned;
  Session *session;
  std::list<std::string> allowedPaths;
  Wt::WApplication *app;
  long long userId;
};
#endif
