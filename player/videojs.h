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


#ifndef VIDEOJS_H
#define VIDEOJS_H
#include "player.h"

namespace Wt {
class WText;
}

class VideoJS : public Player
{
public:
  VideoJS();
  virtual ~VideoJS();
  virtual void setSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink& path, bool autoPlay = true);
  virtual void addSubtitles(const Wt::WLink &path, std::string name, std::string lang);
  virtual Wt::JSignal< Wt::NoClass >& ended();
  virtual void play();
  virtual bool playing();
  virtual void stop();
  virtual Wt::WWidget* widget();
private:
  Wt::WText *m_widget;
  Wt::JSignal<Wt::NoClass> m_ended;
  bool m_playing;
  bool has_subtitles;
  Wt::WString m_text;
};

#endif // VIDEOJS_H
