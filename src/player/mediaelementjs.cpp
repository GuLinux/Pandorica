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
#include "utils/utils.h"

#include <player/private/html5player_p.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <Wt/WPushButton>

using namespace Wt;
using namespace std;
using namespace WtCommons;

MediaElementJs::~MediaElementJs()
{
}
MediaElementJs::MediaElementJs(HTML5PlayerSetup html5PlayerSetup, WObject* parent)
  : PlayerJavascript(html5PlayerSetup, parent), pureHTML5Js(new PureHTML5Js(html5PlayerSetup, this))
{
  html5PlayerSetup.bindEmpty("media.footer");
  html5PlayerSetup.bindWidget("media.header", WW<WPushButton>("Fullscreen").css("btn btn-block hidden-lg hidden-md").onClick([=](WMouseEvent){
    runJavascript("(new MediaElementPlayer('video')).enterFullScreen();");
  }));
}

string MediaElementJs::resizeJs()
{
  return "$(playerId).mediaelementplayer().resize();";
}


void MediaElementJs::onPlayerReady()
{
  if(html5PlayerSetup.mediaType() == HTML5Player::Audio)
    html5PlayerSetup.resolveWidget("media.header")->addStyleClass("hidden-xs hidden-sm");
  map<string,string> mediaElementOptions = {
    {"AndroidUseNativeControls", "false"},
    {"defaultSeekBackwardInterval", "function(media) { return 5; }"},
    {"defaultSeekForwardInterval", "function(media) { return 5; }"},
  };
  
  // TODO: it's not working again, check why
  if(html5PlayerSetup.defaultTracks()["subtitles"].isValid()) {
      mediaElementOptions["startLanguage"] = (boost::format("'%s'") % html5PlayerSetup.defaultTracks()["subtitles"].lang).str();
  }

  vector<string> tempOptions;
  string mediaElementOptionsString = boost::algorithm::join(
    Utils::transform(mediaElementOptions, tempOptions, [](pair<string,string> o){
    return (boost::format("%s: %s") % o.first % o.second).str();
  }), ", ");

  log("notice") << "player options: " << mediaElementOptionsString;
  string onPlayerReadyJs = JS(
    $('video,audio').mediaelementplayer({%s});
    
    // Adding workaround for mediaelementjs bug https://github.com/johndyer/mediaelement/issues/902
    var captionTracks = $('video,audio')[0].textTracks;
    if(captionTracks == undefined) return;
    for(var i=0; i<captionTracks.length; i++)
      if(captionTracks[i].kind == 'subtitles') captionTracks[i].mode='hidden';
    if($('video').length > 0) {
      $('.mejs-video')[0].addEventListener('dblclick', function(o){
        var player = new MediaElementPlayer('video');
        if(player.isFullScreen) player.exitFullScreen();
        else player.enterFullScreen();
      });
    }
  );
  runJavascript((boost::format(onPlayerReadyJs) % mediaElementOptionsString).str() );
  pureHTML5Js->onPlayerReady();
}

string MediaElementJs::customPlayerHTML()
{
  return {};
}
