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


#include "html5player.h"
#include "wt_helpers.h"
#include <Wt/WApplication>
#include <boost/format.hpp>

using namespace Wt;
using namespace std;
using namespace boost;

void HTML5Player::play()
{
  runJavascript("videoPlayer.play();");
  addListener("ended", s_ended.createCall());
}

void HTML5Player::stop()
{
  runJavascript("videoPlayer.stop();");
}

Wt::JSignal<>& HTML5Player::ended()
{
  return s_ended;
}

Wt::WWidget* HTML5Player::widget()
{
  return this;
}

bool HTML5Player::playing()
{
  return isPlaying;
}

void HTML5Player::addSubtitles(const Wt::WLink& path, std::string name, std::string lang)
{
  tracks["subtitles"].push_back(Track(path.url(), lang, name));
}

void HTML5Player::setSource(Wt::WMediaPlayer::Encoding encoding, const Wt::WLink& path, bool autoPlay)
{
  bindString("video.src", path.url(), Wt::XHTMLUnsafeText);
  string type;
  if(encoding == WMediaPlayer::WEBMV)
    type = "webm";
  if(encoding == WMediaPlayer::OGV)
    type = "ogg";
  if(encoding == WMediaPlayer::M4V)
    type = "mp4";
  bindString("video.mimetype", string("video/") + type, Wt::XHTMLUnsafeText);
  wApp->log("notice") << "Autoplay option: " << autoPlay;
  setCondition("autoplay", autoPlay);
}

HTML5Player::HTML5Player(Wt::WContainerWidget* parent): WTemplate(parent),
  s_ended(this, "playbackEnded"),
  s_playing(this, "playbackStarted")
{
  setTemplateText(HTML(<video id="${player.id}" preload="auto" controls style="background-color: black;"
      ${<if-player_size>}${player_size}${</if-player_size>}
      ${<if-player_class>}${player_class}${</if-player_class>}event
      ${<autoplay>}autoplay${</autoplay>}
      width="640" height="264"
        >
      <source type="${video.mimetype}" src="${video.src}" />
      ${track:subtitles}
    </video>), Wt::XHTMLUnsafeText);
  addFunction("track", [this](WTemplate *t, vector<WString> args, std::ostream &output) {
    string argsJoined;
    for(WString arg: args) argsJoined += (argsJoined.empty() ? "" : ", ") + arg.toUTF8();
    wApp->log("notice") << "template function track() :" << argsJoined;
    WString trackType = args[0];
    vector<Track> tracksForType = tracks[trackType.toUTF8()];
    for(Track track: tracksForType)
      output << WString(HTML(<track kind="{4}" src="{1}" srclang="{2}" label="{3}" />\n))
      .arg(track.src).arg(track.lang).arg(track.label).arg(trackType);
    return true;
  });
  bindString("player.id",  playerId());
  s_playing.connect([this](_n6){
    isPlaying = true;
    wApp->log("notice") << "*********** isPlaying: " << isPlaying;
  });
  s_ended.connect([this](_n6) {
    isPlaying = false;
    wApp->log("notice") << "*********** isPlaying: " << isPlaying;
  });
  addListener("play", s_playing.createCall());
}

void HTML5Player::runJavascript(string js)
{
  string runJS = WString(JS(
    var videoPlayer = document.getElementById('{1}');
    {2}
  )).arg(playerId()).arg(js).toUTF8();
  doJavaScript(runJS);
}

void HTML5Player::addListener(string eventName, string function)
{
  string js = WString(JS(
    videoPlayer.addEventListener('{1}', function() { {2} });
  )).arg(eventName).arg(function).toUTF8();
  runJavascript(js);
}

void HTML5Player::setPlayerSize(int width, int height)
{
  string js = (boost::format("videoPlayer.width=%d;") % width).str();
  if(height>0)
    js += (boost::format("videoPlayer.height=%d;") % height).str();
  runJavascript(js);
}



string HTML5Player::playerId()
{
  return string("player_id") + id();
}
