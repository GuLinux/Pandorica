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




#ifndef HTML5PLAYER_H
#define HTML5PLAYER_H

#include "player.h"
#include <Wt/WTemplate>
#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"
#include <functional>
#define MINIMUM_DESKTOP_SIZE 980

class HTML5Player : public Player, public Wt::WContainerWidget
{
public:
    enum MediaType { Video, Audio };
    enum SubType { PureHTML5, MediaElementJs, VideoJs };
    HTML5Player(SubType subType = MediaElementJs, Wt::WContainerWidget* parent = 0);
    virtual ~HTML5Player();
    
    virtual void play();
    virtual void stop();
    virtual Wt::JSignal<>& ended();
    virtual Wt::WWidget* widget();
    virtual bool playing();
    virtual void refresh();
    
    virtual void addSource(const Source& source);
    virtual void setPoster(const Wt::WLink& poster);
    virtual void addSubtitles(const Track& track);
    virtual void setAutoplay(bool autoplay);
    
    virtual void pause();
private:
  D_PTR;
  friend class PlayerJavascript;
};

struct HTML5PlayerSetup {
  std::function<std::string()> playerId;
  std::function<std::string()> templateWidgetId;
  std::function<std::string()> templateWidgetJSRef;
  std::function<void(const std::string&)> bindEmpty;
  std::function<void(const std::string&, Wt::WWidget *)> bindWidget;
  std::function<void(const std::string&)> doJavascript;
  std::function<HTML5Player::MediaType()> mediaType;
  std::function<Wt::WWidget*(const std::string &)> resolveWidget;
};

class PlayerJavascript : public Wt::WObject {
public:
  PlayerJavascript(HTML5PlayerSetup html5PlayerSetup, Wt::WObject* parent = 0);
  virtual void onPlayerReady() = 0;
  virtual std::string customPlayerHTML() = 0;
  virtual std::string resizeJs() = 0;
  void runJavascript(std::string js);
protected:
  HTML5PlayerSetup html5PlayerSetup;
};

#endif // HTML5PLAYER_H

