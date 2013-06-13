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


#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <Wt/WPanel>
#include <boost/filesystem.hpp>
#include "media.h"

namespace PandoricaPrivate {
  class PlaylistPrivate;
}

class Playlist : public Wt::WPanel
{

public:
  Playlist(Session *session, Wt::WContainerWidget* parent = 0);
  virtual ~Playlist();
  void queue(Media media);
  void nextItem(WWidget* itemToPlay = 0);
  Wt::Signal<Media> &next();
  Media first();
  void reset();
private:
  PandoricaPrivate::PlaylistPrivate *const d;
};

#endif // PLAYLIST_H
