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


#ifndef MEDIACOLLECTIONBROWSER_H
#define MEDIACOLLECTIONBROWSER_H

#include <Wt/WContainerWidget>
#include "mediacollection.h"

class Session;
namespace PandoricaPrivate {
  class MediaCollectionBrowserPrivate;
}
class MediaCollection;
class Settings;

class MediaCollectionBrowser : public Wt::WContainerWidget
{

public:
  MediaCollectionBrowser(MediaCollection *collection, Settings *settings, Session *session, Wt::WContainerWidget* parent = 0);
  virtual ~MediaCollectionBrowser();
  Wt::Signal<Media> &queue();
  Wt::Signal<Media> &play();
  bool currentDirectoryHas(Media &media) const;
  void reload();
private:
  PandoricaPrivate::MediaCollectionBrowserPrivate *const d;
};

#endif // MEDIACOLLECTIONBROWSER_H
