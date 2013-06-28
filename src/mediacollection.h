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


#ifndef MEDIACOLLECTION_H
#define MEDIACOLLECTION_H

#include <Wt/WObject>
#include <Wt/WSignal>
#include <boost/filesystem.hpp>
#include "media.h"

class Settings;
namespace Wt {
namespace Dbo {
class Transaction;
}
}

class Session;
namespace PandoricaPrivate {
  class MediaCollectionPrivate;
}
class MediaCollection : public Wt::WObject
{
public:
  MediaCollection(Settings* settings, Session* session, Wt::WApplication* parent = 0);
    virtual ~MediaCollection();
    void rescan(Wt::Dbo::Transaction& transaction);
    std::map<std::string,Media> collection() const;
    Media media(std::string uid) const;
    Wt::Signal<> &scanned();
    void setUserId(long long userId);
    long long viewingAs() const;
    std::vector<Media> sortedMediasList() const;
private:
  PandoricaPrivate::MediaCollectionPrivate *const d;
};

#endif // MEDIACOLLECTION_H
