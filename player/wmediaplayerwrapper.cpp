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
#include <Wt/WApplication>

using namespace Wt;
using namespace boost;
using namespace std;

using MediaType = pair<WMediaPlayer::Encoding,WMediaPlayer::MediaType>;
using EncodingEntry = pair<string,MediaType>;

map<string,MediaType> encodings {
  EncodingEntry("video/webm", MediaType(WMediaPlayer::WEBMV, WMediaPlayer::Video) ),
  EncodingEntry("video/ogg", MediaType(WMediaPlayer::OGV, WMediaPlayer::Video)),
  EncodingEntry("video/mp4", MediaType(WMediaPlayer::M4V, WMediaPlayer::Video)),
  EncodingEntry("video/x-flv", MediaType(WMediaPlayer::FLV, WMediaPlayer::Video)),
  EncodingEntry("audio/mpeg", MediaType(WMediaPlayer::MP3, WMediaPlayer::Audio)),
  EncodingEntry("audio/ogg", MediaType(WMediaPlayer::OGA, WMediaPlayer::Audio))
};

WMediaPlayerWrapper::WMediaPlayerWrapper()
{
//   player = new WMediaPlayer(WMediaPlayer::Video);
}

WMediaPlayerWrapper::~WMediaPlayerWrapper()
{
  delete player;
}

JSignal< NoClass >& WMediaPlayerWrapper::ended()
{
  return player->ended();
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

void WMediaPlayerWrapper::addSubtitles(const Track& track)
{
  wApp->log("notice") << "Add subtitles on WMediaPlayer is unsupported";
}

void WMediaPlayerWrapper::setPoster(const Wt::WLink& poster)
{
}


void WMediaPlayerWrapper::addSource(const Source& source)
{
  if(!player)
    player = new WMediaPlayer(encodings[source.type].second);
  player->addSource(encodings[source.type].first, source.src);
}

void WMediaPlayerWrapper::setAutoplay(bool autoplay)
{
  if(autoplay)
    WTimer::singleShot(1000, [this](WMouseEvent){
      player->play();
    });
}

