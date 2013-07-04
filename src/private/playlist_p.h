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


#ifndef PLAYLIST_PRIVATE
#define PLAYLIST_PRIVATE
#include <Wt/WSignal>
#include "media.h"

namespace Wt {
class WWidget;
class WContainerWidget;
}

namespace PandoricaPrivate {
  typedef std::pair<Wt::WWidget*,Media> QueueItem;
  
  class PlaylistPrivate {
  public:
    PlaylistPrivate(Session *session);
    Session *session;
    std::vector<QueueItem> internalQueue;
    Wt::Signal<Media> next;
    Wt::WContainerWidget *container;
  };
}
#endif
