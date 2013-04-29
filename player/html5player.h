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


#ifndef HTML5PLAYER_H
#define HTML5PLAYER_H

#include "player.h"
#include <Wt/WTemplate>

typedef std::function<void(double currentTime, double totalDuration, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass)> GetCurrentTimeCallback;

class HTML5Player : public Player, public Wt::WTemplate
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
};

#endif // HTML5PLAYER_H
