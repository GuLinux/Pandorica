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




#ifndef PLAYER_H
#define PLAYER_H
#include <Wt/WJavaScript>
#include <Wt/WMediaPlayer>
#include "media.h"

namespace Wt {
class WWidget;
}


struct Track {
  std::string src;
  std::string lang;
  std::string label;
  bool isValid() { !src.empty() && !lang.empty() && !label.empty(); }
  bool operator == (Track &other) { return other.src == src && other.label == label && other.lang == lang; }
};


struct Source {
  std::string src;
  std::string type;
  bool operator == (Source &other) { return other.src == src && other.type == type; }
};

class Player
{
public:
    Player()=default;
    virtual ~Player() {};
    virtual void addSource(const Source &source) = 0;
    virtual void setAutoplay(bool autoplay) = 0;
    virtual void addSubtitles(const Track &track) = 0;
    virtual void setPoster(const Wt::WLink& poster) = 0;
    virtual bool playing() = 0;
    virtual Wt::WWidget *widget() = 0;
    virtual Wt::JSignal <Wt::NoClass >& ended() = 0;
    virtual void stop() = 0;
    virtual void play() = 0;
    virtual void refresh() = 0;
    virtual void setPlayerSize(int width, int height = -1) = 0;
};

#endif // PLAYER_H
