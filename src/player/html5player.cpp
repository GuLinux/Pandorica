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




#include "html5player.h"
#include <utils.h>
#include "Wt-Commons/wt_helpers.h"

#include <Wt/WApplication>
#include <Wt/WAnchor>
#include <Wt/WTimer>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>


using namespace Wt;
using namespace std;
using namespace boost;
using namespace WtCommons;

HTML5Player::HTML5Player(Wt::WContainerWidget* parent)
  : WContainerWidget(parent), s_ended(this, "playbackEnded"), s_playing(this, "playbackStarted"),
  s_playerReady(this, "playbackReady"), s_currentTime(this, "currentTime"), resizeSignal(this, "resizeSignal")
{
  templateWidget = new WTemplate();
  templateWidget->setTemplateText(wtr("html5player.mediatag"), Wt::XHTMLUnsafeText);
  templateWidget->addFunction("sources", [this](WTemplate *t, vector<WString> args, std::ostream &output) {
    for(Source source: sources) {
      output << wtr("player.source").arg(source.type).arg(source.src);
    }
    return true;
  });
  templateWidget->addFunction("track", [this](WTemplate *t, vector<WString> args, std::ostream &output) {
    WString trackType = args[0];
    vector<Track> tracksForType = tracks[trackType.toUTF8()];
    for(Track track: tracksForType) {
      output << wtr("player.track").arg(track.src).arg(track.lang).arg(track.label).arg(trackType).arg("");
    }
    return true;
  });
  templateWidget->bindString("player.id",  playerId());
  s_playing.connect([=](_n6){
    isPlaying = true;
  });
  s_ended.connect([=](_n6) {
    isPlaying = false;
  });
  addListener("play", s_playing.createCall());
  addListener("ended", s_ended.createCall());
  s_playerReady.connect(this, &HTML5Player::playerReady);
  runJavascript(s_playerReady.createCall());
  templateWidget->bindEmpty("poster_option");
  
  WContainerWidget *templateContainer = new WContainerWidget();
  templateContainer->addWidget(templateWidget);
  WContainerWidget *resizeLinks = new WContainerWidget();
  
  
  resizeSignal.connect([=](int size, _n5) {
    templateWidget->doJavaScript(
      (boost::format(" $('#%s').width('%d%%'); \
      $('#%s').mediaelementplayer().resize();")
        % templateContainer->id()
        % size
        % playerId()
      ).str()
    );
  });
  
  resizeLinks->addWidget(WW<WAnchor>("#", "Small").padding(10).onClick([=](WMouseEvent) { resizeSignal.emit(30); } ));
  resizeLinks->addWidget(WW<WAnchor>("#", "Medium").padding(10).onClick([=](WMouseEvent) { resizeSignal.emit(50); } ));
  resizeLinks->addWidget(WW<WAnchor>("#", "Large").padding(10).onClick([=](WMouseEvent) { resizeSignal.emit(70); } ));
  resizeLinks->addWidget(WW<WAnchor>("#", "Full").padding(10).onClick([=](WMouseEvent) { resizeSignal.emit(100); } ));
  templateContainer->setMargin(WLength::Auto, Side::Left | Side::Right);
  addWidget(resizeLinks);
  addWidget(templateContainer);
}

HTML5Player::~HTML5Player()
{
  runJavascript("mediaPlayer.src(""); mediaPlayer.erase();");
}

void HTML5Player::setPoster(const WLink& poster)
{
  templateWidget->bindString("poster_option", string("poster='") + poster.url() + "'");
}



void HTML5Player::play()
{
  runJavascript("mediaPlayer.play();");
}

void HTML5Player::stop()
{
  runJavascript("mediaPlayer.stop();");
}
void HTML5Player::pause()
{
  runJavascript("mediaPlayer.pause();");
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

void HTML5Player::addSubtitles(const Track& track)
{
  tracks["subtitles"].push_back(track);
  if(!defaultTracks["subtitles"].isValid())
    defaultTracks["subtitles"] = track;
}


void HTML5Player::addSource(const Source& source)
{
  if(source.type.find("video/") != string::npos) {
    templateWidget->bindString("media.defaultsize", "width=\"640\" height=\"264\"");
    templateWidget->bindString("media.tagtype",  "video" );
  } else {
    templateWidget->bindString("media.tagtype", "audio" );
    templateWidget->bindString("media.defaultsize", "");
  }
  sources.push_back(source);
}

void HTML5Player::setAutoplay(bool autoplay)
{
  templateWidget->setCondition("autoplay", autoplay);
}


void HTML5Player::playerReady()
{
  
  map<string,string> mediaElementOptions = {
    {"AndroidUseNativeControls", "false"}
  };
  
  // works in theory, but it goes with double subs on chrome 
  if(defaultTracks["subtitles"].isValid() && false) {
      mediaElementOptions["startLanguage"] = (boost::format("'%s'") % defaultTracks["subtitles"].lang).str();
  }
  
  
  string mediaElementOptionsString = boost::algorithm::join(Utils::transform(mediaElementOptions, vector<string>{}, [](pair<string,string> o){
    return (boost::format("%s: %s") % o.first % o.second).str();
  }), ", ");

  log("notice") << "player options: " << mediaElementOptionsString;
  runJavascript((
    boost::format("$('video,audio').mediaelementplayer({%s}); \
    var resizePlayer = function(newSize) { %s}; \
    if(document.width > 600)\
      resizePlayer(60); ")
    % mediaElementOptionsString
    % resizeSignal.createCall("newSize")
  ).str() );
  // doesn't work properly without user interaction
  if(false) {
    runJavascript(JS(
      mediaPlayer.player.enterFullScreen();
    ));
  }
}


void HTML5Player::runJavascript(string js)
{
  string runJS = WString(JS(
    var mediaPlayer = document.getElementById('{1}');
    {2}
  )).arg(playerId()).arg(js).toUTF8();
  doJavaScript(runJS);
}

void HTML5Player::addListener(string eventName, string function)
{
  string js = WString(JS(
    mediaPlayer.addEventListener('{1}', function() { {2} });
  )).arg(eventName).arg(function).toUTF8();
  runJavascript(js);
}

void HTML5Player::setPlayerSize(int width, int height)
{
  string js = JS(
    var currentWidth = mediaPlayer.width;
    var currentHeight = mediaPlayer.height;
    var newWidth = %d;
    var newHeight = %d;
    if(newHeight<1)
      newHeight = newWidth / ( currentWidth / currentHeight );
    mediaPlayer.width=newWidth;
    mediaPlayer.height=newHeight;
  );
  
  js = (boost::format(js) % width % height).str();
  runJavascript(js);
}

void HTML5Player::refresh()
{
  runJavascript(string("$('video,audio').mediaelementplayer();"));
}


string HTML5Player::playerId()
{
  return string("player_id") + templateWidget->id();
}

void HTML5Player::getCurrentTime(GetCurrentTimeCallback callback)
{
  s_currentTime.connect(callback);
  string js = JS(
    var mediaCurrentTime = mediaPlayer.currentTime;
    var mediaTotalTime = mediaPlayer.duration;
    %s ;
  );
  runJavascript( (boost::format(js) % s_currentTime.createCall("mediaCurrentTime", "mediaTotalTime")).str() );
}

