/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  Marco Gulino <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


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
  virtual void addSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink &path);
  virtual bool playing();
  virtual Wt::WWidget *widget();
  virtual Wt::JSignal <Wt::NoClass >& ended();
  virtual void stop();
  virtual void play();
  virtual void addSubtitles(const Wt::WLink &path, std::string name, std::string lang);

private:
  Wt::WMediaPlayer *player;
};

#endif // WMEDIAPLAYERWRAPPER_H