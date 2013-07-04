/*
 * Copyright (c) year Marco Gulino <marco.gulino@gmail.com>
 *
 * This file is part of Pandorica: https://github.com/GuLinux/Pandorica
 *
 * Pandorica is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details (included the COPYING file).
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mediaelementjs.h"
#include "purehtml5js.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils.h"
#include <private/html5player_p.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <Wt/WPushButton>

using namespace Wt;
using namespace std;
using namespace WtCommons;

MediaElementJs::~MediaElementJs()
{
}
MediaElementJs::MediaElementJs(PandoricaPrivate::HTML5PlayerPrivate*const d, WObject* parent)
  : PlayerJavascript(d, parent), pureHTML5Js(new PureHTML5Js(d, this))
{
  d->templateWidget->bindEmpty("media.footer");
  d->templateWidget->bindWidget("media.header", WW<WPushButton>("Fullscreen").css("btn btn-block hidden-desktop").onClick([=](WMouseEvent){
    runJavascript("(new MediaElementPlayer('video')).enterFullScreen();");
  }));
}

string MediaElementJs::resizeJs()
{
  return "$(playerId).mediaelementplayer().resize();";
}


void MediaElementJs::onPlayerReady()
{
  if(d->sources[0].type.find("video/") == string::npos)
    d->templateWidget->resolveWidget("media.header")->addStyleClass("hidden-phone hidden-tablet");
  map<string,string> mediaElementOptions = {
    {"AndroidUseNativeControls", "false"}
  };
  // works in theory, but it goes with double subs on chrome
  if(d->defaultTracks["subtitles"].isValid() && false) {
      mediaElementOptions["startLanguage"] = (boost::format("'%s'") % d->defaultTracks["subtitles"].lang).str();
  }

  string mediaElementOptionsString = boost::algorithm::join(
    Utils::transform(mediaElementOptions, vector<string>{}, [](pair<string,string> o){
    return (boost::format("%s: %s") % o.first % o.second).str();
  }), ", ");

  log("notice") << "player options: " << mediaElementOptionsString;
  // Adding workaround for mediaelementjs bug https://github.com/johndyer/mediaelement/issues/902
  runJavascript((
    boost::format("$('video,audio').mediaelementplayer({%s});\
    var captionTracks = $('video,audio')[0].textTracks; \
      for(var i=0; i<captionTracks.length; i++) \
        if(captionTracks[i].kind == 'subtitles') captionTracks[i].mode='hidden'; \
          if($('video').length > 0) { \
            $('video')[0].addEventListener('dblclick', function(o){\
              var player = new MediaElementPlayer('video');\
              if(player.isFullScreen) player.exitFullScreen();\
              else player.enterFullScreen();\
            }); \
          }\
    ")
    % mediaElementOptionsString
  ).str() );
  pureHTML5Js->onPlayerReady();
}

string MediaElementJs::customPlayerHTML()
{
  return {};
}
