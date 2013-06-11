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

typedef std::function<void(double currentTime, double totalDuration, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass)> GetCurrentTimeCallback;

class HTML5Player : public Player, public Wt::WContainerWidget
{

public:
    HTML5Player(Wt::WContainerWidget* parent = 0);
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
    virtual void setPlayerSize(int width, int height = -1);
    
    virtual void pause();
    void getCurrentTime(GetCurrentTimeCallback callback);
protected:
  std::string playerId();
  virtual void runJavascript(std::string js);
  virtual void addListener(std::string eventName, std::string function);
  virtual void playerReady();
private:
    Wt::JSignal<> s_ended;
    Wt::JSignal<> s_playing;
    Wt::JSignal<> s_playerReady;
    Wt::JSignal<double, double> s_currentTime;
    std::map<std::string, std::vector<Track>> tracks;
    std::vector<Source> sources;
    bool isPlaying = false;
    std::map<std::string,Track> defaultTracks;
    Wt::WTemplate *templateWidget;
    Wt::JSlot resizeSlot;
    Wt::JSlot scrollSlot;
};

#endif // HTML5PLAYER_H
