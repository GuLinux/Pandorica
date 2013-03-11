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


#include "wmediaplayerwrapper.h"
#include <Wt/WMediaPlayer>
#include <Wt/WTimer>

using namespace Wt;
using namespace boost;
using namespace std;

WMediaPlayerWrapper::WMediaPlayerWrapper()
{
  player = new WMediaPlayer(WMediaPlayer::Video);
}

WMediaPlayerWrapper::~WMediaPlayerWrapper()
{
  delete player;
}

JSignal< NoClass >& WMediaPlayerWrapper::ended()
{
  return player->ended();
}

void WMediaPlayerWrapper::setSource(WMediaPlayer::Encoding encoding, const WLink& path, bool autoPlay)
{
  player->addSource(encoding, path);
  if(autoPlay)
    WTimer::singleShot(1000, [this](WMouseEvent){
      player->play();
    });
}

bool WMediaPlayerWrapper::playing()
{
  return player->playing();
}

WWidget* WMediaPlayerWrapper::widget()
{
  return player;
}

void WMediaPlayerWrapper::stop()
{
  player->stop();
}

void WMediaPlayerWrapper::play()
{
  player->play();
}

void WMediaPlayerWrapper::addSubtitles(const WLink& path, string name, string lang)
{

}

