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
  virtual void setPlayerSize(int width, int height = -1) {}
private:
  Wt::WText *m_widget;
  Wt::JSignal<Wt::NoClass> m_ended;
  bool m_playing;
  bool has_subtitles;
  Wt::WString m_text;
};

#endif // VIDEOJS_H
