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


#ifndef PLAYER_H
#define PLAYER_H
#include <Wt/WJavaScript>
#include <Wt/WMediaPlayer>

namespace Wt {
class WWidget;
}

class Player
{

public:
    Player()=default;
    virtual ~Player() {};
    virtual void setSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink &path, bool autoPlay = true) = 0;
    virtual void addSubtitles(const Wt::WLink &path, std::string name, std::string lang) = 0;
    virtual bool playing() = 0;
    virtual Wt::WWidget *widget() = 0;
    virtual Wt::JSignal <Wt::NoClass >& ended() = 0;
    virtual void stop() = 0;
    virtual void play() = 0;
};

#endif // PLAYER_H
