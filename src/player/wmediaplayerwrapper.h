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





#ifndef WMEDIAPLAYERWRAPPER_H
#define WMEDIAPLAYERWRAPPER_H
#include "player.h"

namespace Wt {
class WMediaPlayer;
}

class WMediaPlayerWrapper : public Player
{

public:
  WMediaPlayerWrapper();
  virtual ~WMediaPlayerWrapper();
  virtual bool playing();
  virtual Wt::WWidget *widget();
  virtual Wt::JSignal <Wt::NoClass >& ended();
  virtual void stop();
  virtual void play();
  virtual void addSubtitles(const Track& track);
  virtual void setPoster(const Wt::WLink& poster);
  virtual void refresh();
  // TODO
  virtual void addSource(const Source& source);
  virtual void setAutoplay(bool autoplay);
  virtual void setPlayerSize(int width, int height = -1);
private:
  Wt::WMediaPlayer *player = 0;
};

#endif // WMEDIAPLAYERWRAPPER_H
