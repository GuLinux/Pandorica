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

HTML5Player::HTML5Player(Wt::WContainerWidget* parent)
  : WTemplate(parent), s_ended(this, "playbackEnded"), s_playing(this, "playbackStarted"), s_playerReady(this, "playbackReady")
{
  setTemplateText(WString::tr("html5player.videotag"), Wt::XHTMLUnsafeText);
  addFunction("sources", [this](WTemplate *t, vector<WString> args, std::ostream &output) {
    for(Source source: sources) {
      output << WString::tr("player.source").arg(source.type).arg(source.src);
    }
    return true;
  });
  addFunction("track", [this](WTemplate *t, vector<WString> args, std::ostream &output) {
    WString trackType = args[0];
    vector<Track> tracksForType = tracks[trackType.toUTF8()];
    for(Track track: tracksForType) {
      output << WString::tr("player.track").arg(track.src).arg(track.lang).arg(track.label).arg(trackType).arg("");
    }
    return true;
  });
  bindString("player.id",  playerId());
  s_playing.connect([this](_n6){
    isPlaying = true;
  });
  s_ended.connect([this](_n6) {
    isPlaying = false;
  });
  addListener("play", s_playing.createCall());
  addListener("ended", s_ended.createCall());
  s_playerReady.connect(this, &HTML5Player::playerReady);
  runJavascript(s_playerReady.createCall());
}

HTML5Player::~HTML5Player()
{
  runJavascript("videoPlayer.src(""); videoPlayer.erase();");
}


void HTML5Player::play()
{
  runJavascript("videoPlayer.play();");
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
  Track track = Track(path.url(), lang, name);
  tracks["subtitles"].push_back(track);
  if(!defaultTracks["subtitles"].isValid())
    defaultTracks["subtitles"] = track;
}

void HTML5Player::addSubtitles(const Track& track)
{

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
  Source source{path.url(), string("video/") + type};
  addSource(source);
  setAutoplay(autoPlay);
}

void HTML5Player::addSource(const Source& source)
{
  sources.push_back(source);
}

void HTML5Player::setAutoplay(bool autoplay)
{
  setCondition("autoplay", autoplay);
}


void HTML5Player::playerReady()
{
  
  string mediaelementOptions = HTML(
    iPadUseNativeControls: true,
    iPhoneUseNativeControls: true, 
    AndroidUseNativeControls: true
  );
  /* works in theory, but it goes with double subs on chrome
   */
  if(defaultTracks["subtitles"].isValid()) {
    mediaelementOptions += mediaelementOptions.empty() ? "" : ", " + string("startLanguage: '") + defaultTracks["subtitles"].lang + "'";
  }
  runJavascript(string("$('video,audio').mediaelementplayer({ ") + mediaelementOptions + " });");
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


