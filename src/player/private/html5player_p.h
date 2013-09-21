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


#ifndef HTML5PLAYER_P_H
#define HTML5PLAYER_P_H
#include <Wt/WSignal>
#include <Wt/WJavaScriptSlot>
#include <Wt/WJavaScript>
#include <player/player.h>
#include <player/html5player.h>

class HTML5Player::Private {
public:
  Private(HTML5Player *q);
  Wt::JSignal<> ended;
  Wt::JSignal<> playing;
  Wt::JSignal<> playerReady;
  Wt::JSignal<double, double> currentTime;
  std::map<std::string, std::vector<Track>> tracks;
  std::vector<Source> sources;
  bool isPlaying = false;
  std::map<std::string,Track> defaultTracks;
  Wt::WTemplate *templateWidget;
  Wt::JSlot resizeSlot;
  Wt::JSlot scrollSlot;
    Wt::WContainerWidget* resizeLinks;
  
  std::string playerId() const;
  virtual void addListener(std::string eventName, std::string function);
  virtual void playerReadySlot();
  void setZoomScroll();
  std::string linkResizeJS() const;
  PlayerJavascript *playerJavascript;
private:
  HTML5Player *q;
};

#endif